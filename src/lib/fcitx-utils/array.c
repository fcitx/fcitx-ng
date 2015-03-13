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

#include <stdlib.h>
#include "utils.h"

typedef struct _FcitxPtrArrayPrivate FcitxPtrArrayPrivate;
struct _FcitxPtrArrayPrivate
{
    size_t size;
    FcitxPtrArraySizeGrowFunc sizeGrowFunc;
    FcitxDestroyNotify freeFunc;
    FcitxClosureFunc destoryNotify2;
    void* userData;
};

FCITX_EXPORT_API
FcitxPtrArray* fcitx_ptr_array_new(FcitxDestroyNotify destoryNotify)
{
    return fcitx_ptr_array_new_full(NULL, destoryNotify, NULL, NULL);
}

FCITX_EXPORT_API
FcitxPtrArray* fcitx_ptr_array_new_full(FcitxPtrArraySizeGrowFunc sizeGrowFunc, FcitxDestroyNotify freeFunc, FcitxClosureFunc destoryNotify2, void* userData)
{
    FcitxPtrArray* array = fcitx_utils_new_with_private(FcitxPtrArray);
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    priv->sizeGrowFunc = sizeGrowFunc;
    priv->freeFunc = freeFunc;
    priv->destoryNotify2 = destoryNotify2;
    priv->userData = userData;
    return array;
}

FCITX_EXPORT_API
void fcitx_ptr_array_resize(FcitxPtrArray* array)
{
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    if (priv->sizeGrowFunc) {
         priv->size = priv->sizeGrowFunc(array, priv->size, priv->userData);
    } else {
        priv->size = priv->size ? priv->size * 2 : 1;
    }
    
    array->data = fcitx_utils_realloc(array->data, priv->size * sizeof(void*));
}

FCITX_EXPORT_API
void fcitx_ptr_array_insert(FcitxPtrArray* array, void* data, size_t position)
{
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    if (position > array->len) {
        return;
    }
    
    if (priv->size < array->len + 1) {
        fcitx_ptr_array_resize(array);
    }

    for (size_t i = array->len ; i -- > position; ) {
        array->data[i + 1] = array->data[i];
    }
    
    array->data[position] = data;
    array->len ++;
}

FCITX_EXPORT_API
void fcitx_ptr_array_remove(FcitxPtrArray* array, size_t position, void** steal)
{
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    if (position > array->len) {
        return;
    }
    void* data = array->data[position];
    
    array->len --;
    for (size_t i = position ; i < array->len; i ++) {
        array->data[i] = array->data[i + 1];
    }
    
    if (steal == data) {
        *steal = data;
    } else {
        if (priv->destoryNotify2) {
            priv->destoryNotify2(data, priv->userData);
        }
        if (priv->freeFunc) {
            priv->freeFunc(data);
        }
    }
}

FCITX_EXPORT_API
void fcitx_ptr_array_remove_fast(FcitxPtrArray* array, size_t position, void** steal)
{
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    if (position > array->len) {
        return;
    }
    void* data = array->data[position];
    array->len --;
    array->data[position] = array->data[array->len];
    
    if (steal == data) {
        *steal = data;
    } else {
        if (priv->destoryNotify2) {
            priv->destoryNotify2(data, priv->userData);
        }
        if (priv->freeFunc) {
            priv->freeFunc(data);
        }
    }
}

FCITX_EXPORT_API
void fcitx_ptr_array_sort(FcitxPtrArray* array, FcitxCompareFunc compare)
{
    qsort(array->data, array->len, sizeof(void*), compare);
}

FCITX_EXPORT_API
void fcitx_ptr_array_sort_r(FcitxPtrArray* array, FcitxCompareClosureFunc compare)
{
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    fcitx_qsort_r(array->data, array->len, sizeof(void*), compare, priv->userData);
}

FCITX_EXPORT_API
void fcitx_ptr_array_clear(FcitxPtrArray* array)
{
    FcitxPtrArrayPrivate* priv = FCITX_GET_PRIVATE(array, FcitxPtrArray);
    for (size_t i = 0 ; i < array->len; i ++) {
        if (priv->destoryNotify2) {
            priv->destoryNotify2(array->data[i], priv->userData);
        }
        if (priv->freeFunc) {
            priv->freeFunc(array->data[i]);
        }
    }

    array->len = 0;
}

FCITX_EXPORT_API
void fcitx_ptr_array_free(FcitxPtrArray* array)
{
    if (!array) {
        return;
    }
    fcitx_ptr_array_clear(array);

    free(array->data);
    free(array);
}
