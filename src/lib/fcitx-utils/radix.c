#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "stringutils.h"
#include "utils.h"
#include "radix.h"
#include "list.h"

/**
 * radix tree is simply a compressed trie with common prefix
 * it behaves like a dictionary supporting prefix matching
 * 
 * the implementation here is to support dynamic update
 */

typedef struct _FcitxRadixTreeNode FcitxRadixTreeNode;

struct _FcitxRadixTreeNode
{
    char* key;
    FcitxRadixTreeNode** subNodes;
    void* data;
    size_t nodeSize;
} ;

struct _FcitxRadixTree
{   FcitxRadixTreeNode* root;
    size_t maxKeyLength;
    int size;
    FcitxClosureFunc destroyNotify;
    void* userData;
};

static FcitxRadixTreeNode** fcitx_radix_tree_find(FcitxRadixTree* radix, const char* key, FcitxRadixTreeNode** * parent, const char** keyOff, char** nodeKeyOff);


FcitxRadixTreeNode* fcitx_radix_tree_node_new(const char* key)
{
    FcitxRadixTreeNode* node = fcitx_utils_new(FcitxRadixTreeNode);
    node->key = key ? strdup(key) : NULL;
    return node;
}

void fcitx_radix_tree_node_free(FcitxRadixTree* radix, FcitxRadixTreeNode* node, FcitxClosureFunc destroyNotify)
{
    if (node->data && destroyNotify) {
        destroyNotify(node->data, radix->userData);
    }
    
    for (size_t i = 0; i < node->nodeSize; i ++ ) {
        fcitx_radix_tree_node_free(radix, node->subNodes[i], destroyNotify);
    }
    
    free(node->key);
    free(node->subNodes);
    free(node);
}

FCITX_EXPORT_API
FcitxRadixTree* fcitx_radix_tree_new(FcitxClosureFunc destroyNotify, void* data)
{
    FcitxRadixTree* radix = fcitx_utils_new(FcitxRadixTree);
    radix->destroyNotify = destroyNotify;
    radix->root = fcitx_radix_tree_node_new(NULL);
    radix->userData = data;
    return radix;
}

FCITX_EXPORT_API
void fcitx_radix_tree_free(FcitxRadixTree* radix)
{
    fcitx_radix_tree_node_free(radix, radix->root, radix->destroyNotify);
    free(radix);
}

/**
 * lookup the key for a single vertex, if it key doesnt exists, returns NULL
 */
FcitxRadixTreeNode** fcitx_radix_tree_node_get(FcitxRadixTree* tree, FcitxRadixTreeNode* node, char key)
{
    if (!node->subNodes) {
        return NULL;
    }
    
    // use bsearch might be too expensive here
    int lo = 0;
    int hi = node->nodeSize - 1;
    while (hi >= lo) {
        int mid = (lo + hi) / 2;
        if (node->subNodes[mid]->key[0] == key) {
            return &node->subNodes[mid];
        } else if (node->subNodes[mid]->key[0] < key) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    
    return NULL;
}

int fcitx_radix_tree_node_cmp(const void* a, const void* b) {
    FcitxRadixTreeNode* const* nodea = a, * const*nodeb = b;
    return (*nodea)->key[0] - (*nodeb)->key[0];
}

void fcitx_radix_tree_node_add_subnode(FcitxRadixTree* radix, FcitxRadixTreeNode* parent, FcitxRadixTreeNode* sub)
{
    parent->nodeSize ++;
    parent->subNodes = realloc(parent->subNodes, parent->nodeSize * sizeof(FcitxRadixTreeNode*));
    parent->subNodes[parent->nodeSize - 1] = sub;
    qsort(parent->subNodes, parent->nodeSize, sizeof(FcitxRadixTreeNode*), fcitx_radix_tree_node_cmp);
}

boolean fcitx_radix_tree_store(FcitxRadixTree* radix, const char* key, void* data)
{
    FcitxRadixTreeNode** node = NULL;
    char* nodeKey = NULL;
    FcitxRadixTreeNode** subNode = fcitx_radix_tree_find(radix, key, &node, &key, &nodeKey);
    
    if (!subNode && !nodeKey) {
        FcitxRadixTreeNode *newNode = fcitx_radix_tree_node_new(key);
        fcitx_radix_tree_node_add_subnode(radix, *node, newNode);
        newNode->data = data;
        return true;
    }
    
    if (subNode) {
        if ((*subNode)->data) {
            return false;
        } else {
            (*subNode)->data = data;
        }
    } else if (*nodeKey) {
        FcitxRadixTreeNode *branchNode = fcitx_radix_tree_node_new(NULL), *oldNode = *node;
        *node = branchNode;
        branchNode->key = oldNode->key;
        oldNode->key = strdup(nodeKey);
        fcitx_radix_tree_node_add_subnode(radix, branchNode, oldNode);
        *nodeKey = '\0';
        ssize_t len = nodeKey - branchNode->key + 1;
        branchNode->key = realloc(branchNode->key, len * sizeof(char));
        if (*key) {
            FcitxRadixTreeNode* newNode = fcitx_radix_tree_node_new(key);
            newNode->data = data;
            fcitx_radix_tree_node_add_subnode(radix, branchNode, newNode);
        } else {
            branchNode->data = data;
        }
    }
    return true;
}

FCITX_EXPORT_API
boolean fcitx_radix_tree_add(FcitxRadixTree* radix, const char* key, void* leaf)
{
    if (fcitx_radix_tree_store(radix, key, leaf)) {
        size_t len = strlen(key);
        if (len > radix->maxKeyLength) {
            radix->maxKeyLength = len;
        }
        radix->size ++;
        return true;
    }
    return false;
}

FcitxRadixTreeNode** fcitx_radix_tree_node_get_first(FcitxRadixTreeNode* node)
{
    if (node->nodeSize > 0) {
        return &node->subNodes[0];
    }
    
    return NULL;
}

void fcitx_radix_tree_node_merge(FcitxRadixTree* radix, FcitxRadixTreeNode** node)
{
    FcitxRadixTreeNode* sub2 = *fcitx_radix_tree_node_get_first(*node);
    char* mergedkey = NULL;
    fcitx_utils_alloc_cat_str(mergedkey, (*node)->key, sub2->key);
    (*node)->nodeSize = 0;
    fcitx_radix_tree_node_free(radix, *node, 0);

    *node = sub2;
    free(sub2->key);
    sub2->key = mergedkey;
}

FCITX_EXPORT_API
boolean fcitx_radix_tree_remove(FcitxRadixTree* radix, const char* key)
{
    FcitxRadixTreeNode** node = NULL;
    
    FcitxRadixTreeNode** subNode = fcitx_radix_tree_find(radix, key, &node, NULL, NULL);
    
    if (subNode && (*subNode)->data) {
        if (radix->destroyNotify) {
            radix->destroyNotify((*subNode)->data, radix->userData);
        }
        (*subNode)->data = NULL;
        
        // if the dict is a, ab, and remove a.
        if ((*subNode)->nodeSize == 0) {
            fcitx_radix_tree_node_free(radix, *subNode, NULL);
            memmove(subNode, subNode + 1, (&((*node)->subNodes[(*node)->nodeSize]) - &subNode[1]) * sizeof(FcitxRadixTreeNode*));
            (*node)->nodeSize --;
            
            if ((*node)->nodeSize == 1 && node != &radix->root && (*node)->data == NULL) {
                fcitx_radix_tree_node_merge(radix, node);
            } else {
                if ((*node)->nodeSize == 0) {
                    free((*node)->subNodes);
                    (*node)->subNodes = NULL;
                } else {
                    (*node)->subNodes = realloc((*node)->subNodes, sizeof(FcitxRadixTreeNode*) * (*node)->nodeSize);
                }
            }
        } else if ((*subNode)->nodeSize == 1) {
            // if dict contains a, ab, abc, and remove ab.
            fcitx_radix_tree_node_merge(radix, subNode);
        }
        radix->size --;
        return true;
    }
    return false;
}

void fcitx_radix_tree_foreach_node(FcitxRadixTree* radix, char* key, size_t len, FcitxRadixTreeNode* node, FcitxRadixTreeForeachCallback callback, void* data)
{
    if (node->data) {
        callback(key, node->data, data);
    }
    
    
    for (size_t i = 0; i < node->nodeSize; i ++ ) {
        int j = 0;
        while (node->subNodes[i]->key[j]) {
            key[len + j] = node->subNodes[i]->key[j];
            j++;
        }
        key[len + j] = '\0';
        fcitx_radix_tree_foreach_node(radix, key, len + j, node->subNodes[i], callback, data);
    }
    key[len] = '\0';
}

FCITX_EXPORT_API
size_t fcitx_radix_tree_size(FcitxRadixTree* radix)
{
    return radix->size;
}

FCITX_EXPORT_API
void fcitx_radix_tree_foreach(FcitxRadixTree* radix, const char* key, FcitxRadixTreeForeachCallback callback, void* data)
{
    if (!key) {
        key = "";
    }
    const char* keyOff = NULL;
    char* nodeKeyOff = NULL;
    FcitxRadixTreeNode** node = NULL;    
    FcitxRadixTreeNode** subNode = fcitx_radix_tree_find(radix, key, &node, &keyOff, &nodeKeyOff);
    
    // there's no prefix
    if (*keyOff) {
        return;
    }
    
    char* currentKey = fcitx_utils_newv(char, radix->maxKeyLength + 1);
    int len;
    if (!nodeKeyOff) {
        len = keyOff - key;
        strncpy(currentKey, key, len);
    } else {
        size_t remainlen = strlen(nodeKeyOff);
        len = (keyOff - key) + remainlen;
        strncpy(currentKey, key, keyOff - key);
        strncpy(&currentKey[keyOff - key], nodeKeyOff, remainlen);
    }
    currentKey[len] = '\0';
        
    fcitx_radix_tree_foreach_node(radix, currentKey, len, subNode ? *subNode : *node, callback, data);
    free(currentKey);
}

void* fcitx_radix_tree_exact_match(FcitxRadixTree* radix, const char* key)
{
    FcitxRadixTreeNode** node = fcitx_radix_tree_find(radix, key, NULL, NULL, NULL);
    if (node) {
        return (*node)->data;
    }
    return NULL;
}

FcitxRadixTreeNode** fcitx_radix_tree_find(FcitxRadixTree* radix, const char* key, FcitxRadixTreeNode*** parent, const char** keyOff, char** nodeKeyOff)
{
    FcitxRadixTreeNode** node = &radix->root;
    FcitxRadixTreeNode** subNode = NULL;
    char* nodeKey;
    while (true) {
        nodeKey = NULL;
        subNode = fcitx_radix_tree_node_get(radix, *node, *key);
        if (!subNode) {
            break;
        }
        
        nodeKey = (*subNode)->key;
        while(*key && *key == *nodeKey) {
            key++;
            nodeKey++;
        }
        
        // exact match
        if (*key == '\0' && *nodeKey == '\0') {
            nodeKey = NULL;
            break;
        }
        
        node = subNode;
        subNode = NULL;
        
        if (*nodeKey) {
            break;
        }
    }
    
    if (parent) {
        *parent = node;
    }
    if (keyOff) {
        *keyOff = key;
    }
    if (nodeKeyOff) {
        *nodeKeyOff = nodeKey;
    }
    return subNode;
}