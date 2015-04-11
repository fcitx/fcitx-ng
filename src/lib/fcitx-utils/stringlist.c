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

#include <stdarg.h>
#include <stdio.h>
#include "utils.h"

/* last we pre-define a few icd for common utarrays of ints and strings */
static void utarray_str_cpy(void *dst, const void *src)
{
    char **_src = (char**)src, **_dst = (char**)dst;
    *_dst = (*_src == NULL) ? NULL : fcitx_utils_strdup(*_src);
}
static void utarray_str_dtor(void *elt)
{
    char **eltc = (char**)elt;
    if (*eltc)
        free(*eltc);
}

static const UT_icd ut_str_icd = {
    sizeof(char*), NULL, utarray_str_cpy, utarray_str_dtor
};

FCITX_EXPORT_API const UT_icd *const fcitx_str_icd = &ut_str_icd;

FCITX_EXPORT_API FcitxStringList*
fcitx_utils_string_list_new()
{
    FcitxStringList *array = utarray_new(fcitx_str_icd);
    return array;
}

FCITX_EXPORT_API FcitxStringList*
fcitx_utils_string_list_append_split_full(FcitxStringList *list,
                                const char* str, const char *delm, bool keepEmpty)
{
    const char *src = str;
    const char *pos;
    size_t len;
    while ((len = strcspn(src, delm)), *(pos = src + len)) {
        if (keepEmpty || len) {
            fcitx_utils_string_list_append_len(list, src, len);
        }
        src = pos + 1;
    }
    if (len)
        fcitx_utils_string_list_append_len(list, src, len);
    return list;
}

FCITX_EXPORT_API
FcitxStringList* fcitx_utils_string_split_full(const char* str, const char* delm, bool keepEmpty)
{
    FcitxStringList* array = fcitx_utils_string_list_new();
    return fcitx_utils_string_list_append_split_full(array, str, delm, keepEmpty);
}

FCITX_EXPORT_API
void fcitx_utils_string_list_printf_append(FcitxStringList* list, const char* fmt,...)
{
    char* buffer;
    va_list ap;
    va_start(ap, fmt);
    fcitx_vasprintf(&buffer, fmt, ap);
    va_end(ap);
    fcitx_utils_string_list_append_no_copy(list, buffer);
}

FCITX_EXPORT_API
char* fcitx_utils_string_list_join(FcitxStringList* list, char delm)
{
    if (!list)
        return NULL;

    if (utarray_len(list) == 0)
        return fcitx_utils_strdup("");

    size_t len = 0;
    char** str;
    for (str = (char**) utarray_front(list);
         str != NULL;
         str = (char**) utarray_next(list, str))
    {
        len += strlen(*str) + 1;
    }

    char* result = (char*)fcitx_utils_malloc(sizeof(char) * len);
    char* p = result;
    for (str = (char**) utarray_front(list);
         str != NULL;
         str = (char**) utarray_next(list, str))
    {
        size_t strl = strlen(*str);
        memcpy(p, *str, strl);
        p += strl;
        *p = delm;
        p++;
    }
    result[len - 1] = '\0';

    return result;
}

FCITX_EXPORT_API
int fcitx_utils_string_list_contains(FcitxStringList* list, const char* scmp)
{
    char** str;
    for (str = (char**) utarray_front(list);
         str != NULL;
         str = (char**) utarray_next(list, str))
    {
        if (strcmp(scmp, *str) == 0)
            return 1;
    }
    return 0;
}

FCITX_EXPORT_API
void fcitx_utils_string_list_free(FcitxStringList* list)
{
    if (!list) {
        return;
    }
    utarray_free(list);
}

FCITX_EXPORT_API FcitxStringList*
fcitx_utils_string_list_append_no_copy(FcitxStringList *list, char *str)
{
    utarray_extend_back(list);
    *(char**)utarray_back(list) = str;
    return list;
}

FCITX_EXPORT_API FcitxStringList*
fcitx_utils_string_list_append_len(FcitxStringList *list, const char *str, size_t len)
{
    char *buff = fcitx_utils_set_str_with_len(NULL, str, len);
    fcitx_utils_string_list_append_no_copy(list, buff);
    return list;
}
