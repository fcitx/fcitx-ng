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

#ifndef _FCITX_UTILS_ARRAY_H_
#define _FCITX_UTILS_ARRAY_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include "types.h"

typedef struct _FcitxPtrArray FcitxPtrArray;
typedef size_t (*FcitxPtrArraySizeGrowFunc)(FcitxPtrArray* array, size_t oldSize, void* userData);
typedef void* (*FcitxPtrArrayInitElementCallback)(size_t index, void* userData);

struct _FcitxPtrArray
{
    void** data;
    size_t len;
};

FcitxPtrArray* fcitx_ptr_array_new_full(FcitxPtrArraySizeGrowFunc sizeGrowFunc, FcitxDestroyNotify destoryNotify, FcitxClosureFunc destoryNotify2, void* userData);
FcitxPtrArray* fcitx_ptr_array_new(FcitxDestroyNotify destoryNotify);
void fcitx_ptr_array_sort(FcitxPtrArray* array, FcitxCompareFunc compare);
void fcitx_ptr_array_sort_r(FcitxPtrArray* array, FcitxCompareClosureFunc compare);
void fcitx_ptr_array_insert(FcitxPtrArray* array, void* data, size_t position);
void fcitx_ptr_array_free(FcitxPtrArray* array);
void fcitx_ptr_array_clear(FcitxPtrArray* array);

void fcitx_ptr_array_remove(FcitxPtrArray* array, size_t position, void** steal);
void fcitx_ptr_array_remove_fast(FcitxPtrArray* array, size_t position, void** steal);
void fcitx_ptr_array_set(FcitxPtrArray* array, size_t index, void* data);
void fcitx_ptr_array_resize(FcitxPtrArray* array, size_t size, FcitxPtrArrayInitElementCallback callback, void* userData);

static _FCITX_ALWAYS_INLINE_ size_t fcitx_ptr_array_size(FcitxPtrArray* array)
{
    return array->len;
}

#define fcitx_ptr_array_index(array, index, type) ((type) _fcitx_ptr_array_index((array), (index)))

static _FCITX_ALWAYS_INLINE_ void* _fcitx_ptr_array_index(FcitxPtrArray* array, size_t index)
{
    return index >= array->len ? NULL : array->data[index];
}

static _FCITX_ALWAYS_INLINE_ void fcitx_ptr_array_prepend(FcitxPtrArray* array, void* data)
{
    fcitx_ptr_array_insert(array, data, 0);
}

static _FCITX_ALWAYS_INLINE_ void fcitx_ptr_array_append(FcitxPtrArray* array, void* data)
{
    fcitx_ptr_array_insert(array, data, array->len);
}

static _FCITX_ALWAYS_INLINE_ void fcitx_ptr_array_pop(FcitxPtrArray* array, void** steal)
{
    fcitx_ptr_array_remove(array, array->len - 1, steal);
}

#endif // _FCITX_UTILS_ARRAY_H_
