#ifndef _FCITX_UTILS_MACRO_INTERNAL_H
#define _FCITX_UTILS_MACRO_INTERNAL_H

#define FCITX_REFCOUNT_FUNCTION_DEFINE(TYPENAME, SLUGNAME) \
FCITX_EXPORT_API \
TYPENAME* SLUGNAME##_ref(TYPENAME* data) \
{ \
    fcitx_utils_atomic_add (&data->refcount, 1); \
    return data; \
} \
FCITX_EXPORT_API \
void SLUGNAME##_unref(TYPENAME* data) \
{ \
    int32_t oldvalue = fcitx_utils_atomic_add (&data->refcount, -1); \
    if (oldvalue == 1) { \
        SLUGNAME##_free(data); \
    } \
}

#endif // _FCITX_UTILS_MACRO_INTERNAL_H
