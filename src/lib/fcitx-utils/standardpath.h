/***************************************************************************
 *   Copyright (C) 2014~2014 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef _FCITX_STANDARD_PATH_H_
#define _FCITX_STANDARD_PATH_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include "macro.h"
#include "types.h"
#include "stringlist.h"
#include "dict.h"

FCITX_DECL_BEGIN

typedef struct _FcitxStandardPath FcitxStandardPath;

typedef struct _FcitxStandardPathFile
{
    FILE* fp;
    char* path;
} FcitxStandardPathFile;

typedef enum _FcitxStandardPathType
{
    FSPT_Config,
    FSPT_Data,
    FSPT_Cache,
    FSPT_Runtime,
} FcitxStandardPathType;

typedef enum _FcitxStandardPathFilterFlag
{
    FSPFT_Writable = (1 << 0),
    FSPFT_Append = (1 << 1),
    FSPFT_Prefix = (1 << 2),
    FSPFT_Suffix = (1 << 3),
    FSPFT_Callback = (1 << 4),
    FSPFT_Sort = (1 << 5),
    FSPFT_LocateAll = (1 << 6),
    FSPFT_Write = FSPFT_Writable | FSPFT_Append,
} FcitxxStandardPathFilterFlag;

typedef bool (*FcitxxStandardPathFilterCallback)(const char* path, void* data);

typedef struct _FcitxStandardPathFilter
{
    uint32_t flag;
    FcitxxStandardPathFilterCallback callback;
    void* userData;
    char* suffix;
    char* prefix;
} FcitxStandardPathFilter;

FcitxStandardPath* fcitx_standard_path_new();

FcitxStandardPathFile* fcitx_standard_path_locate(FcitxStandardPath* sp, FcitxStandardPathType type, const char* path, uint32_t flag);

FcitxDict* fcitx_standard_path_match(FcitxStandardPath* sp, FcitxStandardPathType type, const char* path, FcitxStandardPathFilter* filter);

FcitxStandardPath* fcitx_standard_path_ref(FcitxStandardPath* sp);

void fcitx_standard_path_unref(FcitxStandardPath* sp);

void fcitx_standard_path_file_close(FcitxStandardPathFile* file);

FCITX_DECL_END

#endif
