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
#include <fcitx-utils/utarray.h>

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
FcitxStringList* fcitx_utils_string_split(const char *str, char delm);

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

char*
fcitx_utils_set_str_with_len(char *res, const char *str, size_t len);

FcitxStringList *fcitx_utils_string_list_append_no_copy(FcitxStringList *list, char *str);
FcitxStringList *fcitx_utils_string_list_append_len(FcitxStringList *list,
                                                    const char *str, size_t len);
FcitxStringList *fcitx_utils_append_split_string(FcitxStringList *list, const char* str,
                                                 const char *delm);

FCITX_DECL_END

#endif
