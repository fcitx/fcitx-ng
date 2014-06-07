#ifndef _FCITX_UTILS_DICT_H_
#define _FCITX_UTILS_DICT_H_

#include <fcitx-utils/macro.h>
#include <fcitx-utils/types.h>
#include <stddef.h>
#include <string.h>

FCITX_DECL_BEGIN

typedef struct _FcitxDict FcitxDict;
typedef void (*FcitxDictForeachFunc)(const char* key, void* data, void* arg);

FcitxDict* fcitx_dict_new(FcitxDestroyNotify freeFunc);

size_t fcitx_dict_size(FcitxDict* dict);

void fcitx_dict_steal_all(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_foreach(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

boolean fcitx_dict_insert(FcitxDict* dict, const char* key, size_t keyLen, void* value, boolean replace);

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
static inline boolean fcitx_dict_insert_by_str(FcitxDict* dict, const char* key, void* value, boolean replace)
{
    return fcitx_dict_insert(dict, key, strlen(key), value, replace);
}

#define fcitx_dict_insert_data_by_str(DICT, KEY, VALUE, REPLACE) \
    fcitx_dict_insert_by_str(DICT, KEY, (void*)((intptr_t)(VALUE)), REPLACE);
#define fcitx_dict_insert_data(DICT, KEY, KEYLEN, VALUE) \
    fcitx_dict_insert(DICT, KEY, KEYLEN, (void*)((intptr_t)(VALUE)), REPLACE);

boolean fcitx_dict_lookup(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut);
static inline boolean fcitx_dict_lookup_by_str(FcitxDict* dict, const char* key, void** dataOut)
{
    return fcitx_dict_lookup(dict, key, strlen(key), dataOut);
}

boolean fcitx_dict_remove(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut);

static inline boolean fcitx_dict_remove_by_str(FcitxDict* dict, const char* key, void** dataOut)
{
    return fcitx_dict_remove(dict, key, strlen(key), dataOut);
}

void fcitx_dict_remove_all(FcitxDict* dict);

void fcitx_dict_free(FcitxDict* dict);

FCITX_DECL_END

#endif
