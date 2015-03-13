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

#ifndef _FCITX_LIBRARY_H_
#define _FCITX_LIBRARY_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stdlib.h>
#include "macro.h"
#include "types.h"

typedef enum _FcitxLibraryLoadHint
{
    FLLH_ResolveAllSymbolsHint = 0x1,
    FLLH_PreventUnloadHint = 0x2,
    FLLH_ExportExternalSymbolsHint = 0x4
} FcitxLibraryLoadHint;

typedef struct _FcitxLibrary FcitxLibrary;
typedef void (*FcitxLibraryDataParser)(const char* data, void* arg);

FcitxLibrary* fcitx_library_new(const char* path);
void fcitx_library_free(FcitxLibrary* lib);

bool fcitx_library_load(FcitxLibrary* lib, uint32_t hint);
bool fcitx_library_unload(FcitxLibrary* lib);
void* fcitx_library_resolve(FcitxLibrary* lib, const char* symbol);
bool fcitx_library_find_data(FcitxLibrary* lib, const char* slug, const char* magic, size_t lenOfMagic, FcitxLibraryDataParser parser, void* arg);
const char* fcitx_library_error(FcitxLibrary* lib);

#endif // _FCITX_LIBRARY_H_
