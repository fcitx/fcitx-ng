/*
 * Copyright (C) 2015~2015 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

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

typedef struct _FcitxDictData {
    char* key;
    size_t keyLen;
    void* data;
} FcitxDictData;

FcitxDict* fcitx_dict_new(FcitxDestroyNotify freeFunc);

FcitxDict* fcitx_dict_clone(FcitxDict* other, FcitxDictCopyFunc copyFunc);

size_t fcitx_dict_size(FcitxDict* dict);

void fcitx_dict_steal_all(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_foreach(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_remove_if(FcitxDict* dict, FcitxDictForeachFunc func, void* data);

void fcitx_dict_remove_data(FcitxDict* dict, FcitxDictData* data, void** dataOut);

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

static inline bool fcitx_dict_insert_by_data(FcitxDict* dict, intptr_t i, void* value, bool replace)
{
    return fcitx_dict_insert(dict, (char*) &i, sizeof(i), value, replace);
}

#define fcitx_dict_insert_data_by_str(DICT, KEY, VALUE, REPLACE) \
    fcitx_dict_insert_by_str(DICT, KEY, (void*)((intptr_t)(VALUE)), REPLACE);
#define fcitx_dict_insert_data(DICT, KEY, KEYLEN, VALUE, REPLACE) \
    fcitx_dict_insert(DICT, KEY, KEYLEN, (void*)((intptr_t)(VALUE)), REPLACE);

#define fcitx_dict_lookup(dict, key, keyLen, dataOut) (_fcitx_dict_lookup(dict, key, keyLen, (void**) dataOut))
#define fcitx_dict_lookup_by_str(dict, key, dataOut) (_fcitx_dict_lookup_by_str(dict, key, (void**) dataOut))
#define fcitx_dict_lookup_by_data(dict, key, dataOut) (_fcitx_dict_lookup_by_data(dict, key, (void**) dataOut))

bool _fcitx_dict_lookup(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut);
static inline bool _fcitx_dict_lookup_by_str(FcitxDict* dict, const char* key, void** dataOut)
{
    return _fcitx_dict_lookup(dict, key, strlen(key), dataOut);
}

static inline bool _fcitx_dict_lookup_by_data(FcitxDict* dict, intptr_t i, void** dataOut)
{
    return _fcitx_dict_lookup(dict, (char*) &i, sizeof(i), dataOut);
}

bool fcitx_dict_remove(FcitxDict* dict, const char* key, size_t keyLen, void** dataOut);

static inline bool fcitx_dict_remove_by_str(FcitxDict* dict, const char* key, void** dataOut)
{
    return fcitx_dict_remove(dict, key, strlen(key), dataOut);
}

static inline bool fcitx_dict_remove_by_data(FcitxDict* dict, intptr_t i, void** dataOut)
{
    return fcitx_dict_remove(dict, (char*) &i, sizeof(i), dataOut);
}

void fcitx_dict_sort(FcitxDict* dict, FcitxDictCompareFunc compare, void* userData);

void fcitx_dict_remove_all(FcitxDict* dict);

void fcitx_dict_free(FcitxDict* dict);

FcitxDictData* fcitx_dict_first(FcitxDict* dict);

FcitxDictData* fcitx_dict_data_next(FcitxDictData* data);


FCITX_DECL_END

#endif
