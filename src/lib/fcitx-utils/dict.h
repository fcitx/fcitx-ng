#ifndef _FCITX_UTILS_DICT_H_
#define _FCITX_UTILS_DICT_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stddef.h>
#include <string.h>

#include "macro.h"
#include "types.h"

FCITX_DECL_BEGIN

typedef struct _FcitxDict FcitxDict;
typedef bool (*FcitxDictForeachFunc)(const char* key, size_t keyLen, void** data, void* arg);
typedef int (*FcitxDictCompareFunc)(const char* keyA, size_t keyALen, const void* dataA, const char* keyB, size_t keyBLen, const void* dataB, void* userData);
typedef void* (*FcitxDictCopyFunc)(void* data);

FcitxDict* fcitx_dict_new(FcitxDestroyNotify freeFunc);

FcitxDict* fcitx_dict_clone(FcitxDict* other, FcitxDictCopyFunc copyFunc);

size_t fcitx_dict_size(FcitxDict* dict);

void fcitx_dict_steal_all(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_foreach(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_remove_if(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

bool fcitx_dict_insert(FcitxDict* dict, const char* key, size_t keyLen, void* value, bool replace);

/**
 * insert an item into dictionary
 *
 * @param dict dict
 * @param key string key
 * @param keyLen key length
 * @param value data
 * @param replace replace existing key
 * @return sucessfully inserted or not
 */
static inline bool fcitx_dict_insert_by_str(FcitxDict* dict, const char* key, void* value, bool replace)
{
    return fcitx_dict_insert(dict, key, strlen(key), value, replace);
}

#define fcitx_dict_insert_data_by_str(DICT, KEY, VALUE, REPLACE) \
    fcitx_dict_insert_by_str(DICT, KEY, (void*)((intptr_t)(VALUE)), REPLACE);
#define fcitx_dict_insert_data(DICT, KEY, KEYLEN, VALUE) \
    fcitx_dict_insert(DICT, KEY, KEYLEN, (void*)((intptr_t)(VALUE)), REPLACE);

#define fcitx_dict_lookup(dict, key, keyLen, dataOut) _fcitx_dict_lookup(dict, key, keyLen, (void**) dataOut)
#define fcitx_dict_lookup_by_str(dict, key, dataOut) _fcitx_dict_lookup_by_str(dict, key, (void**) dataOut)

bool _fcitx_dict_lookup(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut);
static inline bool _fcitx_dict_lookup_by_str(FcitxDict* dict, const char* key, void** dataOut)
{
    return _fcitx_dict_lookup(dict, key, strlen(key), dataOut);
}

bool fcitx_dict_remove(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut);

static inline bool fcitx_dict_remove_by_str(FcitxDict* dict, const char* key, void** dataOut)
{
    return fcitx_dict_remove(dict, key, strlen(key), dataOut);
}

void fcitx_dict_sort(FcitxDict* dict, FcitxDictCompareFunc compare, void* userData);

void fcitx_dict_remove_all(FcitxDict* dict);

void fcitx_dict_free(FcitxDict* dict);

FCITX_DECL_END

#endif
