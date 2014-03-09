#ifndef _FCITX_UTILS_TYPES_H_
#define _FCITX_UTILS_TYPES_H_

#include <fcitx-utils/macro.h>
#include <stdint.h>
#include <stddef.h>

FCITX_DECL_BEGIN

/**
 * fcitx boolean
 **/
typedef int32_t boolean;
#if !defined(__cplusplus) && !defined(FCITX_DONOT_DEFINE_TRUE_FALSE)
/**
 * fcitx true
 */
#define true (1)
/**
 * fcitx false
 */
#define false (0)
#endif

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
