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

#ifndef _FCITX_LINKED_LIST_H_
#define _FCITX_LINKED_LIST_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stdlib.h>
#include "macro.h"
#include "types.h"

FCITX_DECL_BEGIN

typedef struct _FcitxListHead
{
    struct _FcitxListHead* prev;
    struct _FcitxListHead* next;
} FcitxListHead;

static _FCITX_ALWAYS_INLINE_ void fcitx_list_init(FcitxListHead* pos)
{
    pos->next = pos;
    pos->prev = pos;
}

static _FCITX_ALWAYS_INLINE_ bool fcitx_list_is_empty(FcitxListHead* pos)
{
    return pos->next == pos;
}

static _FCITX_ALWAYS_INLINE_ void __fcitx_list_insert_between(FcitxListHead* add, FcitxListHead* prev, FcitxListHead* next)
{
    next->prev = add;
    prev->next = add;
    add->next = next;
    add->prev = prev;
}

static _FCITX_ALWAYS_INLINE_ void fcitx_list_prepend(FcitxListHead* add, FcitxListHead* pos)
{
    __fcitx_list_insert_between(add, pos, pos->next);
}

static _FCITX_ALWAYS_INLINE_ void fcitx_list_append(FcitxListHead* add, FcitxListHead* pos)
{
    __fcitx_list_insert_between(add, pos->prev, pos);
}

static _FCITX_ALWAYS_INLINE_ void fcitx_list_remove(FcitxListHead* pos)
{
    FcitxListHead* next = pos->next;
    FcitxListHead* prev = pos->prev;
    prev->next = next;
    next->prev = prev;

    pos->next = NULL;
    pos->prev = NULL;
}

void fcitx_list_sort(FcitxListHead* list, size_t offset, FcitxCompareFunc compare);
void fcitx_list_sort_r(FcitxListHead* list, size_t offset, FcitxCompareClosureFunc compare, void* data);

#define fcitx_list_foreach(key, head) \
    for (FcitxListHead *key = (head)->next; key != (head); \
         key = key->next)

#define fcitx_list_foreach_safe(key, head) \
    for (FcitxListHead *key = (head)->next, *__n = key->next; key != (head); \
         key = __n, __n = key->next)

#define fcitx_list_foreach_nl(key, head) \
    for (key = (head)->next; key != (head); \
         key = key->next)

#define fcitx_list_foreach_safe_nl(key, __n, head) \
    for (key = (head)->next, __n = key->next; key != (head); \
         key = __n, __n = key->next)

#define fcitx_list_entry_foreach(key, type, head, field) \
    for (type* key = fcitx_container_of((head)->next, type, field); \
         &key->field != (head); \
         key = fcitx_container_of(key->field.next, type, field))

#define fcitx_list_entry_foreach_nl(key, type, head, field) \
    for (key = fcitx_container_of((head)->next, type, field); \
         &key->field != (head); \
         key = fcitx_container_of(key->field.next, type, field))

#define fcitx_list_entry_foreach_safe(key, type, head, field) \
    for (type* key = fcitx_container_of((head)->next, type, field), *__n = fcitx_container_of(key->field.next, type, field) ; \
         &key->field != (head); \
         key = __n, __n = fcitx_container_of(key->field.next, type, field))

#define fcitx_list_entry_foreach_safe_nl(key, __n, type, head, field) \
    for (key = fcitx_container_of((head)->next, type, field), __n = fcitx_container_of(key->field.next, type, field) ; \
         &key->field != (head); \
         key = __n, __n = fcitx_container_of(key->field.next, type, field))


FCITX_DECL_END

#endif
