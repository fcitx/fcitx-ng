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

#ifndef _FCITX_UTILS_FS_H_
#define _FCITX_UTILS_FS_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stddef.h>
#include "types.h"
#include "macro.h"

FCITX_DECL_BEGIN

bool fcitx_utils_isdir(const char* path);
bool fcitx_utils_isreg(const char *path);
bool fcitx_utils_islnk(const char *path);

size_t fcitx_utils_clean_path(const char* path, char* buf);
bool fcitx_utils_make_path(const char *path);

FCITX_DECL_END

#endif
