#ifndef _FCITX_UTILS_DICT_H_
#define _FCITX_UTILS_DICT_H_

#include <fcitx-utils/macro.h>
#include <fcitx-utils/types.h>
#include <stddef.h>

FCITX_DECL_BEGIN

typedef struct _FcitxDict FcitxDict;
typedef void (*FcitxDictForeachFunc)(const char* key, void* data, void* arg);

FcitxDict* fcitx_dict_new(FcitxDestroyNotify freeFunc);

size_t fcitx_dict_size(FcitxDict* dict);

boolean fcitx_dict_remove(FcitxDict* dict, const char* key, void** dataOut);

void fcitx_dict_steal_all(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_foreach(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

/**
 * insert an item into dictionary
 * 
 * @param dict dict
 * @param key string key
 * @param value data
 * @param replace replace existing key
 * @return sucessfully inserted or not
 */
boolean fcitx_dict_insert(FcitxDict* dict, const char* key, void* value, boolean replace);

#define fcitx_dict_insert_data(DICT, KEY, VALUE) \
    fcitx_dict_insert(DICT, KEY, (void*)((intptr_t)(VALUE)));

boolean fcitx_dict_lookup(FcitxDict* dict, const char* key, void** dataOut);

void fcitx_dict_free(FcitxDict* dict);

FCITX_DECL_END

#endif
