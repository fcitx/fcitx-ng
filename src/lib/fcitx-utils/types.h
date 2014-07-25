#ifndef _FCITX_UTILS_TYPES_H_
#define _FCITX_UTILS_TYPES_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "macro.h"

FCITX_DECL_BEGIN

typedef enum _FcitxTriState
{
    Tri_False = false,
    Tri_True = true,
    Tri_Unknown
} FcitxTriState;

/**
 * Function used to free the pointer
 **/
typedef void (*FcitxDestroyNotify)(void *p);
typedef void (*FcitxClosureFunc)(void*, void*);
typedef int (*FcitxCompareFunc)(const void*, const void*);
typedef int (*FcitxCompareClosureFunc)(const void*, const void*, void*);

/**
 * Function used to free the content of a structure,
 * DO NOT free the pointer itself
 **/
typedef void (*FcitxCallBack)();

FCITX_DECL_END

#endif
