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
#include <fcitx-utils/types.h>
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
char* fcitx_utils_get_fcitx_path(const char* type);
char* fcitx_utils_get_fcitx_path_with_filename(const char* type, const char* filename);

/**
 * ascii only is lower
 * 
 * @param c char
 * @return boolean
 */
static _FCITX_INLINE_
boolean fcitx_utils_islower(char c)
{
    return c >= 'a' && c <= 'z';
}

/**
 * ascii only is upper
 * 
 * @param c char
 * @return boolean
 */
static _FCITX_INLINE_
boolean fcitx_utils_isupper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static _FCITX_INLINE_
char fcitx_utils_tolower(char c)
{
    return fcitx_utils_isupper(c) ? c - 'A' + 'a' : c;
}

static _FCITX_INLINE_
char fcitx_utils_toupper(char c)
{
    return fcitx_utils_islower(c) ? c - 'a' + 'A' : c;
}

#define FCITX_WHITESPACE "\f\n\r\t\v "

static _FCITX_INLINE_
boolean fcitx_utils_isspace(char c)
{
    return c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == ' ';
}


/**
 * if obj is null, free it, after that, if str is NULL set it with NULL,
 * if str is not NULL, set it with strdup(str)
 *
 * @param obj object string
 * @param str source string
 * @return void
 **/
void fcitx_utils_string_swap(char** obj, const char* str);
void fcitx_utils_string_swap_with_len(char** obj,
                                      const char* str, size_t len);

static _FCITX_INLINE_ uintptr_t
fcitx_utils_align_to(uintptr_t len, uintptr_t align)
{
    uintptr_t left;
    if ((left = len % align))
        return len + align - left;
    return len;
}

void *fcitx_utils_custom_bsearch(const void *key, const void *base,
                                 size_t nmemb, size_t size, int accurate,
                                 int (*compar)(const void *, const void *));

FCITX_DECL_END

#endif
