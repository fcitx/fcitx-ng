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
typedef void (*FcitxInitFunc)(void *p);
typedef void (*FcitxClosureFunc)(void*, void*);
typedef int (*FcitxCompareFunc)(const void*, const void*);
typedef void (*FcitxCopyFunc)(void* dst, const void* src);
typedef int (*FcitxCompareClosureFunc)(const void*, const void*, void*);

/**
 * Function used to free the content of a structure,
 * DO NOT free the pointer itself
 **/
typedef void (*FcitxCallback)();

FCITX_DECL_END

#endif
