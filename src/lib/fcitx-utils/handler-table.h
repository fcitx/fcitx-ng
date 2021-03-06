/*
 * Copyright (C) 2012~2013 by Yichao Yu
 * yyc1992@gmail.com
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

#ifndef _FCITX_UTILS_HANDLER_TABLE_H
#define _FCITX_UTILS_HANDLER_TABLE_H

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include "macro.h"
#include "types.h"
#include <string.h>

FCITX_DECL_BEGIN

/**
 * FcitxHandlerTable is a multi hash table specialized for handler.
 *
 * The key of the handler can be any sequence of bytes, for each key, there is a sequence
 * of values which is usually used as a single handler.
 *
 * For example, if you want to implement a file monitor, then you can use FcitxHandlerTable
 * like this: Let key to be the file name, value to be the handler, then you can define a
 * function to initialize some os specific data/resource for one new file name, and you
 * can get a unique id for each handler (id can be reused once its removed).
 *
 * If one file is not watched anymore, FcitxFreeKeyFunc will be called and you can release
 * any resource you alloced for this file.
 */
typedef struct _FcitxHandlerTable FcitxHandlerTable;
typedef struct _FcitxHandlerKey FcitxHandlerKey;

typedef void (*FcitxFreeKeyFunc)(void *data, const void* key, size_t len, void *owner);
typedef void (*FcitxInitKeyFunc)(void *data, const void* key, size_t len, void *owner);
typedef void (*FcitxHandlerTableForeachKeyCallback)(FcitxHandlerTable* table, FcitxHandlerKey* key, void* data);

typedef struct {
    size_t size;
    FcitxInitKeyFunc init;
    FcitxFreeKeyFunc free;
    void *owner;
} FcitxHandlerKeyDataVTable;

FcitxHandlerTable *fcitx_handler_table_new(size_t obj_size,
                                            FcitxDestroyNotify free_func);
FcitxHandlerTable *fcitx_handler_table_new_with_keydata(
    size_t obj_size, FcitxDestroyNotify free_func,
    const FcitxHandlerKeyDataVTable *key_vtable);
#define fcitx_handler_table_new(obj_size, free_func, args...)   \
    _fcitx_handler_table_new(obj_size, free_func, ##args, NULL)
#define _fcitx_handler_table_new(obj_size, free_func, key_vtable, ...)  \
    (fcitx_handler_table_new_with_keydata)(obj_size, free_func, key_vtable)

int fcitx_handler_table_append(FcitxHandlerTable *table,
                                size_t keysize, const void *key,
                                const void *obj);
int fcitx_handler_table_prepend(FcitxHandlerTable *table,
                                size_t keysize, const void *key,
                                const void *obj);
int fcitx_handler_key_append(FcitxHandlerTable *table,
                                FcitxHandlerKey *key, const void *obj);
int fcitx_handler_key_prepend(FcitxHandlerTable *table,
                                FcitxHandlerKey *key, const void *obj);
bool fcitx_handler_key_is_empty(FcitxHandlerTable* table, FcitxHandlerKey* key);

void *fcitx_handler_table_get_by_id(FcitxHandlerTable *table, int id);

FcitxHandlerKey *fcitx_handler_table_get_key_by_id(FcitxHandlerTable *table, int id);

FcitxHandlerKey *fcitx_handler_table_find_key(
    FcitxHandlerTable *table, size_t keysize,
    const void *key, bool create);
static inline FcitxHandlerKey*
fcitx_handler_table_find_strkey(FcitxHandlerTable *table, const char *key,
                                bool create)
{
    return fcitx_handler_table_find_key(table, strlen(key),
                                        (const void*)key, create);
}
#define fcitx_handler_table_find_key(table, keysize, key, args...)      \
    _fcitx_handler_table_find_key(table, keysize, key, ##args, false)
#define _fcitx_handler_table_find_key(table, keysize, key, create, ...) \
    (fcitx_handler_table_find_key)(table, keysize, key, create)
#define fcitx_handler_table_find_strkey(table, key, args...)            \
    _fcitx_handler_table_find_strkey(table, key, ##args, false)
#define _fcitx_handler_table_find_strkey(table, key, create, ...)       \
    (fcitx_handler_table_find_strkey)(table, key, create)

void *fcitx_handler_key_get_data(FcitxHandlerTable *table,
                                    FcitxHandlerKey *key);
const void *fcitx_handler_key_get_key(
    FcitxHandlerTable *table, FcitxHandlerKey *key, size_t *len);
void *fcitx_handler_key_first(FcitxHandlerTable *table,
                                FcitxHandlerKey *key);
void *fcitx_handler_key_last(FcitxHandlerTable *table,
                                FcitxHandlerKey *key);
int fcitx_handler_key_first_id(FcitxHandlerTable *table,
                                FcitxHandlerKey *key);
int fcitx_handler_key_last_id(FcitxHandlerTable *table,
                                FcitxHandlerKey *key);
void *fcitx_handler_table_first(FcitxHandlerTable *table, size_t keysize,
                                const void *key);
void *fcitx_handler_table_last(FcitxHandlerTable *table, size_t keysize,
                                const void *key);
void *fcitx_handler_table_next(FcitxHandlerTable *table, const void *obj);
void *fcitx_handler_table_prev(FcitxHandlerTable *table, const void *obj);
/* for remove when iterating */
int fcitx_handler_table_first_id(FcitxHandlerTable *table,
                                    size_t keysize, const void *key);
int fcitx_handler_table_last_id(FcitxHandlerTable *table,
                                size_t keysize, const void *key);
int fcitx_handler_table_next_id(FcitxHandlerTable *table, const void *obj);
int fcitx_handler_table_prev_id(FcitxHandlerTable *table, const void *obj);
void fcitx_handler_table_remove_key(FcitxHandlerTable *table,
                                    size_t keysize, const void *key);
void fcitx_handler_table_remove_by_id(FcitxHandlerTable *table, int id);
void fcitx_handler_table_remove_by_id_full(FcitxHandlerTable *table, int id);
void fcitx_handler_table_free(FcitxHandlerTable *table);

void fcitx_handler_table_foreach_key(FcitxHandlerTable* table, FcitxHandlerTableForeachKeyCallback callback, void* userdata);
int fcitx_handler_table_n_key(FcitxHandlerTable* table);

static inline int
fcitx_handler_table_append_strkey(FcitxHandlerTable *table,
                                    const char *keystr, const void *obj)
{
    return fcitx_handler_table_append(table, strlen(keystr),
                                        (const void*)keystr, obj);
}
static inline int
fcitx_handler_table_prepend_strkey(FcitxHandlerTable *table,
                                    const char *keystr, const void *obj)
{
    return fcitx_handler_table_prepend(table, strlen(keystr),
                                        (const void*)keystr, obj);
}
static inline void*
fcitx_handler_table_first_strkey(FcitxHandlerTable *table,
                                    const char *keystr)
{
    return fcitx_handler_table_first(table, strlen(keystr),
                                        (const void*)keystr);
}
static inline void*
fcitx_handler_table_last_strkey(FcitxHandlerTable *table,
                                const char *keystr)
{
    return fcitx_handler_table_last(table, strlen(keystr),
                                    (const void*)keystr);
}
static inline int
fcitx_handler_table_first_id_strkey(FcitxHandlerTable *table,
                                    const char *keystr)
{
    return fcitx_handler_table_first_id(table, strlen(keystr),
                                        (const void*)keystr);
}
static inline int
fcitx_handler_table_last_id_strkey(FcitxHandlerTable *table,
                                    const char *keystr)
{
    return fcitx_handler_table_last_id(table, strlen(keystr),
                                        (const void*)keystr);
}
static inline void
fcitx_handler_table_remove_key_strkey(FcitxHandlerTable *table,
                                        const char *keystr)
{
    fcitx_handler_table_remove_key(table, strlen(keystr),
                                    (const void*)keystr);
}

FCITX_DECL_END

#endif
