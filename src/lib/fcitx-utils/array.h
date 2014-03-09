#ifndef ARRAY_H
#define ARRAY_H

#include <fcitx-utils/types.h>


typedef struct _FcitxPtrArray FcitxPtrArray;
typedef size_t (*FcitxPtrArraySizeGrowFunc)(FcitxPtrArray* array, size_t oldSize, void* userData);

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

void fcitx_ptr_array_remove(FcitxPtrArray* array, size_t position, void** steal);
void fcitx_ptr_array_remove_fast(FcitxPtrArray* array, size_t position, void** steal);

static _FCITX_ALWAYS_INLINE_ size_t fcitx_ptr_array_size(FcitxPtrArray* array)
{
    return array->len;
}

static _FCITX_ALWAYS_INLINE_ void* fcitx_ptr_array_index(FcitxPtrArray* array, size_t index)
{
    return index >= array->len ? NULL : array->data[index];
}

static _FCITX_ALWAYS_INLINE_ void fcitx_ptr_array_prepend(FcitxPtrArray* array, void* data)
{
    fcitx_ptr_array_insert(array, data, 0);
}

static _FCITX_ALWAYS_INLINE_ void fcitx_ptr_array_pop(FcitxPtrArray* array, void** steal)
{
    fcitx_ptr_array_remove(array, array->len - 1, steal);
}

#endif // ARRAY_H