#include <stdlib.h>
#include "utils.h"
#include "uthash.h"

typedef struct _FcitxDictItem {
    FcitxDictData content;
    UT_hash_handle hh;
} FcitxDictItem;

struct _FcitxDict {
    FcitxDictItem* head;
    FcitxDestroyNotify destroyNotify;
};

FCITX_EXPORT_API
FcitxDict* fcitx_dict_new(FcitxDestroyNotify freeFunc)
{
    FcitxDict* dict = fcitx_utils_new(FcitxDict);
    if (!dict) {
        return NULL;
    }

    dict->destroyNotify = freeFunc;
    return dict;
}

FCITX_EXPORT_API
bool fcitx_dict_insert(FcitxDict* dict, const char* key, size_t keyLen, void* value, bool replace)
{
    FcitxDictItem* item = NULL;
    HASH_FIND(hh, dict->head, key, keyLen, item);
    if (item) {
        if (replace) {
            if (dict->destroyNotify) {
                dict->destroyNotify(item->content.data);
            }
            item->content.data = value;
            return true;
        } else {
            return false;
        }
    }

    item = fcitx_utils_new(FcitxDictItem);
    if (!item) {
        return false;
    }

    // make all key null terminated
    item->content.key = malloc(keyLen + 1);
    item->content.key[keyLen] = 0;
    item->content.keyLen = keyLen;
    memcpy(item->content.key, key, keyLen);
    item->content.data = value;
    HASH_ADD_KEYPTR(hh, dict->head, item->content.key, keyLen, item);
    return true;
}

FCITX_EXPORT_API
bool _fcitx_dict_lookup(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut)
{
    FcitxDictItem* item = NULL;
    HASH_FIND(hh, dict->head, key, keyLen, item);

    if (item == NULL) {
        return false;
    }

    if (dataOut) {
        *dataOut = item->content.data;
    }
    return true;
}

FCITX_EXPORT_API
size_t fcitx_dict_size(FcitxDict* dict)
{
    return HASH_COUNT(dict->head);
}

// pass arg as NULL can avoid real data being free'd
void fcitx_dict_item_free(FcitxDictItem* item, FcitxDict* arg)
{
    if (arg && arg->destroyNotify) {
        arg->destroyNotify(item->content.data);
    }
    free(item->content.key);
    free(item);
}

typedef struct {
    FcitxDictCompareFunc callback;
    void* userData;
} fcitx_dict_compare_context;

int fcitx_dict_item_compare(FcitxDictItem* a, FcitxDictItem* b, fcitx_dict_compare_context* context)
{
    FcitxDictItem* itemA = a;
    FcitxDictItem* itemB = b;
    return context->callback(itemA->content.key, itemA->content.keyLen, itemA->content.data,
                             itemB->content.key, itemB->content.keyLen, itemB->content.data,
                             context->userData);
}

int fcitx_dict_item_default_compare(const char* keyA, size_t keyALen, const void* dataA, const char* keyB, size_t keyBLen, const void* dataB, void* userData)
{
    FCITX_UNUSED(dataA);
    FCITX_UNUSED(dataB);
    FCITX_UNUSED(userData);
    size_t size = (keyALen < keyBLen) ? keyALen : keyBLen;
    int result = memcmp(keyA, keyB, size);
    if (result == 0) {
        if (keyALen < keyBLen) {
            result = -1;
        } else if (keyALen > keyBLen) {
            result = 1;
        }
    }
    return result;
}

FCITX_EXPORT_API
void fcitx_dict_sort(FcitxDict* dict, FcitxDictCompareFunc compare, void* userData)
{
    if (!compare) {
        compare = fcitx_dict_item_default_compare;
    }
    fcitx_dict_compare_context context;
    context.callback = compare;
    context.userData = userData;
    HASH_SORT(dict->head, fcitx_dict_item_compare, &context);
}

FCITX_EXPORT_API
void fcitx_dict_remove_all(FcitxDict* dict)
{
    while (dict->head) {
        FcitxDictItem* item = dict->head;
        HASH_DEL(dict->head, item);
        fcitx_dict_item_free(item, dict);
    }
}

FCITX_EXPORT_API
bool fcitx_dict_remove(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut)
{
    FcitxDictItem* item = NULL;
    HASH_FIND(hh, dict->head, key, keyLen, item);

    if (item == NULL) {
        return false;
    }

    if (dataOut) {
        *dataOut = item->content.data;
        HASH_DEL(dict->head, item);
        fcitx_dict_item_free(item, NULL);
    } else {
        HASH_DEL(dict->head, item);
        fcitx_dict_item_free(item, dict);
    }
    return true;
}

FCITX_EXPORT_API
void fcitx_dict_steal_all(FcitxDict* dict, FcitxDictForeachFunc func, void* data) {
    while (dict->head) {
        FcitxDictItem* item = dict->head;
        HASH_DEL(dict->head, item);
        func(item->content.key, item->content.keyLen, &item->content.data, data);
        fcitx_dict_item_free(item, NULL);
    }
}

FCITX_EXPORT_API
void fcitx_dict_foreach(FcitxDict* dict, FcitxDictForeachFunc func, void* data)
{
    HASH_FOREACH(item, dict->head, FcitxDictItem) {
        if (func(item->content.key, item->content.keyLen, &item->content.data, data)) {
            break;
        }
    }
}

FCITX_EXPORT_API
void fcitx_dict_remove_if(FcitxDict* dict, FcitxDictForeachFunc func, void* data)
{
    FcitxDictItem* item = dict->head;
    while (item) {
        FcitxDictItem* nextItem = item->hh.next;
        if (func(item->content.key, item->content.keyLen, &item->content.data, data)) {
            HASH_DEL(dict->head, item);
            fcitx_dict_item_free(item, dict);
        }
        item = nextItem;
    }
}

void fcitx_dict_remove_data(FcitxDict* dict, FcitxDictData* data, void** dataOut)
{
    FcitxDictItem* item = (FcitxDictItem*) data;
    HASH_DEL(dict->head, item);

    if (dataOut) {
        *dataOut = item->content.data;
        HASH_DEL(dict->head, item);
        fcitx_dict_item_free(item, NULL);
    } else {
        HASH_DEL(dict->head, item);
        fcitx_dict_item_free(item, dict);
    }
}

FCITX_EXPORT_API
void fcitx_dict_free(FcitxDict* dict)
{
    if (!dict) {
        return;
    }
    fcitx_dict_remove_all(dict);
    free(dict);
}

FCITX_EXPORT_API
FcitxDict* fcitx_dict_clone(FcitxDict* other, FcitxDictCopyFunc copyFunc)
{
    FcitxDict* dict = fcitx_dict_new(other->destroyNotify);

    HASH_FOREACH(item, other->head, FcitxDictItem) {
        void *data;
        if (copyFunc) {
            data = copyFunc(item->content.data);
        } else {
            data = item->content.data;
        }
        fcitx_dict_insert(dict, item->content.key, item->content.keyLen, data, false);
    }

    return dict;
}

FCITX_EXPORT_API
FcitxDictData* fcitx_dict_first(FcitxDict* dict)
{
    return &dict->head->content;
}

FCITX_EXPORT_API
FcitxDictData* fcitx_dict_data_next(FcitxDictData* data)
{
    FcitxDictItem* item = (FcitxDictItem*) data;
    return item->hh.next;
}
