#include "macro.h"
#include "dict.h"
#include "uthash.h"
#include "utils.h"
#include "types.h"
#include <stdlib.h>

typedef struct _FcitxDictItem {
    char* key;
    void* data;
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
boolean fcitx_dict_insert(FcitxDict* dict, const char* key, void* value, boolean replace)
{
    FcitxDictItem* item = NULL;
    HASH_FIND_STR(dict->head, key, item);
    if (item) {
        if (replace) {
            if (dict->destroyNotify) {
                dict->destroyNotify(item->data);
            }
            item->data = value;
            return true;
        } else {
            return false;
        }
    }

    item = fcitx_utils_new(FcitxDictItem);
    if (!item) {
        return false;
    }

    item->key = strdup(key);
    item->data = value;
    HASH_ADD_KEYPTR(hh, dict->head, item->key, strlen(item->key), item);
    return true;
}

FCITX_EXPORT_API
boolean fcitx_dict_lookup(FcitxDict* dict, const char* key, void** dataOut)
{
    FcitxDictItem* item = NULL;
    HASH_FIND_STR(dict->head, key, item);

    if (item == NULL) {
        return false;
    }

    if (dataOut) {
        *dataOut = item->data;
    }
    return true;
}

FCITX_EXPORT_API
size_t fcitx_dict_size(FcitxDict* dict)
{
    return HASH_COUNT(dict->head);
}

FCITX_EXPORT_API
void fcitx_dict_item_free(FcitxDictItem* item, FcitxDict* arg)
{
    if (arg && arg->destroyNotify) {
        arg->destroyNotify(item->data);
    }
    free(item->key);
    free(item);
}

FCITX_EXPORT_API
boolean fcitx_dict_remove(FcitxDict* dict, const char* key, void** dataOut)
{
    FcitxDictItem* item = NULL;
    HASH_FIND_STR(dict->head, key, item);

    if (item == NULL) {
        return false;
    }

    if (dataOut) {
        *dataOut = item->data;
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
        func(item->key, item->data, data);
        fcitx_dict_item_free(item, NULL);
    }
}

FCITX_EXPORT_API
void fcitx_dict_foreach(FcitxDict* dict, FcitxDictForeachFunc func, void* data)
{
    HASH_FOREACH(item, dict->head, FcitxDictItem) {
        func(item->key, item->data, data);
    }
}

FCITX_EXPORT_API
void fcitx_dict_free(FcitxDict* dict)
{
    while (dict->head) {
        FcitxDictItem* item = dict->head;
        HASH_DEL(dict->head, item);
        fcitx_dict_item_free(item, dict);
    }
    free(dict);
}


