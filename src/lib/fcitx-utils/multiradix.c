#include "radix.h"
#include "utils.h"
#include "multiradix.h"
#include "list.h"

struct _FcitxMultiRadixTree
{
    FcitxRadixTree* radix;
    void* userData;
    size_t off;
    size_t size;
    FcitxClosureFunc destroyNotify;
};

typedef struct _FcitxMultiRadixTreeForeachThunk
{
    
    FcitxMultiRadixTree* multiRadix;
    void* data;
    FcitxRadixTreeForeachCallback callback;
} FcitxMultiRadixTreeForeachThunk;

void fcitx_multi_radix_tree_node_free(void* data, void* userData)
{
    FcitxListHead* list = data;
    FcitxMultiRadixTree* multiRadix = userData;
    fcitx_list_foreach_safe(item, list) {
        void* realData = (FcitxListHead*) (((char*)item) - multiRadix->off);
        if (multiRadix->destroyNotify) {
            multiRadix->destroyNotify(realData, multiRadix->userData);
        }
    }
    free(data);
}

FCITX_EXPORT_API
FcitxMultiRadixTree* fcitx_multi_radix_tree_new(size_t off, FcitxClosureFunc destroyNotify, void* data)
{
    FcitxMultiRadixTree* multiRadix = fcitx_utils_new(FcitxMultiRadixTree);
    multiRadix->radix = fcitx_radix_tree_new(fcitx_multi_radix_tree_node_free, multiRadix);
    multiRadix->userData = data;
    multiRadix->destroyNotify = destroyNotify;
    multiRadix->off = off;
    return multiRadix;
}

FCITX_EXPORT_API
void fcitx_multi_radix_tree_free(FcitxMultiRadixTree* multiRadix)
{
    fcitx_radix_tree_free(multiRadix->radix);
    free(multiRadix);
}

FCITX_EXPORT_API
void fcitx_multi_radix_tree_add(FcitxMultiRadixTree* multiRadix, const char* key, void* leaf)
{
    FcitxListHead* list = fcitx_radix_tree_exact_match(multiRadix->radix, key);
    if (!list) {
        list = fcitx_utils_new(FcitxListHead);
        fcitx_list_init(list);
        fcitx_radix_tree_add(multiRadix->radix, key, list);
    }
    FcitxListHead* l = (FcitxListHead*) ((char*)leaf + multiRadix->off);
    fcitx_list_append(l, list);
    multiRadix->size++;
}

FCITX_EXPORT_API
void fcitx_multi_radix_tree_remove(FcitxMultiRadixTree* multiRadix, char* key, void* item)
{
    FcitxListHead* list = fcitx_radix_tree_exact_match(multiRadix->radix, key);
    if (!list) {
        return;
    }
    FcitxListHead* l = (FcitxListHead*) (((char*)item) + multiRadix->off);
    fcitx_list_remove(l);
    if (multiRadix->destroyNotify) {
        multiRadix->destroyNotify(item, multiRadix->userData);
    }
    multiRadix->size --;
    
    if (fcitx_list_is_empty(list)) {
        fcitx_radix_tree_remove(multiRadix->radix, key);
    }
}

FCITX_EXPORT_API
void fcitx_multi_radix_tree_foreach_callback(const char* key, void* data, void* userData)
{
    FcitxMultiRadixTreeForeachThunk* thunk = userData;
    FcitxListHead* list = data;
    fcitx_list_foreach(item, list) {
        void* realData = (FcitxListHead*) (((char*)item) - thunk->multiRadix->off);
        thunk->callback(key, realData, thunk->data);
    }
}

FCITX_EXPORT_API
void fcitx_multi_radix_tree_foreach(FcitxMultiRadixTree* multiRadix, const char* key, FcitxRadixTreeForeachCallback callback, void* data)
{
    FcitxMultiRadixTreeForeachThunk thunk;
    thunk.multiRadix = multiRadix;
    thunk.data = data;
    thunk.callback = callback;
    fcitx_radix_tree_foreach(multiRadix->radix, key, fcitx_multi_radix_tree_foreach_callback, &thunk);
}

FCITX_EXPORT_API
FcitxListHead* fcitx_multi_radix_tree_exact_match(FcitxMultiRadixTree* multiRadix, const char* key)
{
    return fcitx_radix_tree_exact_match(multiRadix->radix, key);
}

FCITX_EXPORT_API
size_t fcitx_multi_radix_tree_size(FcitxMultiRadixTree* multiRadix)
{
    return multiRadix->size;
}
