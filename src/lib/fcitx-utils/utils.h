/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *   Copyright (C) 2011~2012 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *   Copyright (C) 2012~2013 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
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


#ifndef _FCITX_UTILS_UTILS_H_
#define _FCITX_UTILS_UTILS_H_

#include <fcitx-utils/macro.h>
#include <stdint.h>
#include <stddef.h>

FCITX_DECL_BEGIN


/**
 * Malloc and memset all memory to zero
 *
 * @param bytes malloc size
 * @return void* malloced pointer
 **/
void* fcitx_utils_malloc0(size_t bytes);

#define fcitx_utils_new(TYPE) ((TYPE*) fcitx_utils_malloc0(sizeof(TYPE)))
#define fcitx_utils_newv(TYPE, _N) ((TYPE*) fcitx_utils_malloc0(_N*sizeof(TYPE)))

void fcitx_utils_free(void*);

static inline uintptr_t
fcitx_utils_align_to(uintptr_t len, uintptr_t align)
{
    uintptr_t left;
    if ((left = len % align))
        return len + align - left;
    return len;
}

FCITX_DECL_END

#endif
