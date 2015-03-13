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

#ifndef _FCITX_UTILS_STRING_LIST_H_
#define _FCITX_UTILS_STRING_LIST_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include "types.h"
#include "macro.h"
#include "utarray.h"

FCITX_DECL_BEGIN

typedef UT_array FcitxStringList;

/**
 * create empty string list
 *
 * @return FcitxStringList*
 **/
FcitxStringList* fcitx_utils_string_list_new(void);

/**
 * Split a string by delm
 *
 * @param str input string
 * @param delm character as delimiter
 * @return FcitxStringList* a new utarray for store the split string
 **/
FcitxStringList* fcitx_utils_string_split_full(const char *str, const char* delm, bool keepEmpty);

#define fcitx_utils_string_split(_str, _delm) fcitx_utils_string_split_full(_str, _delm, true)

/**
 * append a string with printf format
 *
 * @param list string list
 * @param fmt printf fmt
 * @return void
 **/
void fcitx_utils_string_list_printf_append(FcitxStringList* list, const char* fmt,...);

/**
 * Join string list with delm
 *
 * @param list string list
 * @param delm delm
 * @return char* return string, need to be free'd
 **/
char* fcitx_utils_string_list_join(FcitxStringList* list, char delm);

/**
 * check if a string list contains a specific string
 *
 * @param list string list
 * @param scmp string to compare
 *
 * @return 1 for found, 0 for not found.
 *
 * @since 4.2.5
 */
int fcitx_utils_string_list_contains(FcitxStringList* list, const char* scmp);

/**
 * Helper function for free the SplitString Output
 *
 * @param list the SplitString Output
 * @return void
 * @see fcitx_utils_split_string
 **/
void fcitx_utils_string_list_free(FcitxStringList *list);

FcitxStringList *fcitx_utils_string_list_append_no_copy(FcitxStringList *list, char *str);
FcitxStringList *fcitx_utils_string_list_append_len(FcitxStringList *list,
                                                    const char *str, size_t len);
FcitxStringList *fcitx_utils_string_list_append_split_full(FcitxStringList *list, const char* str,
                                                           const char *delm, bool keepEmpty);
#define fcitx_utils_string_list_append_split(_list, _str, _delm) \
    fcitx_utils_string_list_append_split_full(_list, _str, _delm, true)

#define fcitx_utils_string_list_append_lines(list, str) fcitx_utils_string_list_append_split(list, str, "\n")

FCITX_DECL_END

#endif // _FCITX_UTILS_STRING_LIST_H_
