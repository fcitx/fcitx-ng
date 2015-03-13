/*
 * Copyright (C) 2012~2015 by CSSlayer
 * wengxt@gmail.com
 * Copyright (C) 2012~2013 by Yichao Yu
 * yyc1992@gmail.com
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

#ifndef _FCITX_UTILS_MISC_H_
#define _FCITX_UTILS_MISC_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <malloc.h>
#include "macro.h"
#include "types.h"

FCITX_DECL_BEGIN

void fcitx_utils_closure_free(void* data, void* userData);
char* fcitx_utils_get_fcitx_path(const char* type);
char* fcitx_utils_get_fcitx_path_with_filename(const char* type, const char* filename);

/**
 * ascii only is lower
 * 
 * @param c char
 * @return bool
 */
static inline
bool fcitx_utils_islower(char c)
{
    return c >= 'a' && c <= 'z';
}

/**
 * ascii only is upper
 * 
 * @param c char
 * @return bool
 */
static inline
bool fcitx_utils_isupper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static inline
char fcitx_utils_tolower(char c)
{
    return fcitx_utils_isupper(c) ? c - 'A' + 'a' : c;
}

static inline
char fcitx_utils_toupper(char c)
{
    return fcitx_utils_islower(c) ? c - 'a' + 'A' : c;
}

#define FCITX_WHITESPACE "\f\n\r\t\v "

static inline
bool fcitx_utils_isspace(char c)
{
    return c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == ' ';
}

static inline
bool fcitx_utils_isdigit(char c)
{
    return c >= '0' && c <= '9';
}

static _FCITX_ALWAYS_INLINE_
char* fcitx_utils_strchr0(const char* s, int c)
{
    return (char*)( s ? strchr(s, c) : NULL);
}

static _FCITX_ALWAYS_INLINE_ int fcitx_utils_strcmp0(const char* a, const char* b)
{
    if (a == NULL && b == NULL)
        return 0;
    if (a == NULL && b)
        return -1;
    if (a && b == NULL)
        return 1;
    return strcmp(a, b);
}

/**
 * if obj is not null, free it, after that, if str is NULL set it with NULL,
 * if str is not NULL, set it with fcitx_utils_strdup(str)
 *
 * @param obj object string
 * @param str source string
 * @return void
 **/
void fcitx_utils_string_swap(char** obj, const char* str);
void fcitx_utils_string_swap_with_len(char** obj,
                                      const char* str, size_t len);

static inline uintptr_t
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



/**
 * read a little endian 32bit unsigned int from a file
 *
 * @param fp FILE* to read from
 * @param p return the integer read
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
size_t fcitx_utils_read_uint32(FILE *fp, uint32_t *p);

/**
 * read a little endian 32bit int from a file
 *
 * @param fp FILE* to read from
 * @param p return the integer read
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
static inline size_t
fcitx_utils_read_int32(FILE *fp, int32_t *p)
{
return fcitx_utils_read_uint32(fp, (uint32_t*)p);
}

/**
 * write a little endian 32bit int to a file
 *
 * @param fp FILE* to write to
 * @param i int to write in host endian
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
size_t fcitx_utils_write_uint32(FILE *fp, uint32_t i);

/**
 * write a little endian 32bit unsigned int to a file
 *
 * @param fp FILE* to write to
 * @param i int to write in host endian
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
static inline size_t
fcitx_utils_write_int32(FILE *fp, int32_t i)
{
return fcitx_utils_write_uint32(fp, (uint32_t)i);
}


/**
 * read a little endian 64bit unsigned int from a file
 *
 * @param fp FILE* to read from
 * @param p return the integer read
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
size_t fcitx_utils_read_uint64(FILE *fp, uint64_t *p);

/**
 * read a little endian 64bit int from a file
 *
 * @param fp FILE* to read from
 * @param p return the integer read
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
static inline size_t
fcitx_utils_read_int64(FILE *fp, int64_t *p)
{
return fcitx_utils_read_uint64(fp, (uint64_t*)p);
}

/**
 * write a little endian 64bit int to a file
 *
 * @param fp FILE* to write
 * @param i int to write in host endian
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
size_t fcitx_utils_write_uint64(FILE *fp, uint64_t i);

/**
 * write a little endian 64bit unsigned int to a file
 *
 * @param fp FILE* to write to
 * @param i int to write in host endian
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
static inline size_t
fcitx_utils_write_int64(FILE *fp, int64_t i)
{
return fcitx_utils_write_uint64(fp, (uint64_t)i);
}


/**
 * read a little endian 16bit unsigned int from a file
 *
 * @param fp FILE* to read from
 * @param p return the integer read
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
size_t fcitx_utils_read_uint16(FILE *fp, uint16_t *p);

/**
 * read a little endian 16bit int from a file
 *
 * @param fp FILE* to read from
 * @param p return the integer read
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
static inline size_t
fcitx_utils_read_int16(FILE *fp, int16_t *p)
{
return fcitx_utils_read_uint16(fp, (uint16_t*)p);
}

/**
 * write a little endian 16bit int to a file
 *
 * @param fp FILE* to write to
 * @param i int to write in host endian
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
size_t fcitx_utils_write_uint16(FILE *fp, uint16_t i);

/**
 * write a little endian 16bit unsigned int to a file
 *
 * @param fp FILE* to write to
 * @param i int to write in host endian
 * @return 1 on success, 0 on error
 * @since 4.2.6
 **/
static inline size_t
fcitx_utils_write_int16(FILE *fp, int16_t i)
{
return fcitx_utils_write_uint16(fp, (uint16_t)i);
}

static inline void* fcitx_utils_calloc(size_t nmemb, size_t size)
{
    void* result = calloc(nmemb, size);
    if (!result) {
        abort();
    }
    return result;
}

static inline void* fcitx_utils_malloc(size_t size)
{
    void* result = malloc(size);
    if (!result) {
        abort();
    }
    return result;
}

static inline void* fcitx_utils_realloc(void* ptr, size_t size)
{
    void* result = realloc(ptr, size);
    if (!result) {
        abort();
    }
    return result;
}

static inline void* fcitx_utils_malloc0(size_t size)
{
    return fcitx_utils_calloc(1, size);
}

#define fcitx_utils_new(TYPE) ((TYPE*) fcitx_utils_malloc0(sizeof(TYPE)))
#define fcitx_utils_newv(TYPE, _N) ((TYPE*) fcitx_utils_calloc(_N, sizeof(TYPE)))
#define fcitx_utils_new_with_private(TYPE) ((TYPE*) fcitx_utils_malloc0(sizeof(TYPE) + sizeof(TYPE##Private)))
#define fcitx_utils_free(PTR) free(PTR)

void fcitx_utils_init_as_daemon();

static inline void*
fcitx_utils_malloc_with_data(size_t base_size, const void *extra_data,
                           size_t extra_size)
{
    void *res = fcitx_utils_malloc0(base_size + extra_size);
    memcpy(((char*)res) + base_size, extra_data, extra_size);
    return res;
}

static inline void*
fcitx_utils_malloc_with_str(size_t base_size, const char *extra_str)
{
    return fcitx_utils_malloc_with_data(base_size, (const void*)extra_str,
                                        strlen(extra_str) + 1);
}

#define fcitx_utils_new_with_data(TYPE, data, size)                     \
    ((TYPE*)fcitx_utils_malloc_with_data(sizeof(TYPE), data, size))
#define fcitx_utils_new_with_str(TYPE, str)                     \
    ((TYPE*)fcitx_utils_malloc_with_str(sizeof(TYPE), str))

FCITX_DECL_END

#endif
