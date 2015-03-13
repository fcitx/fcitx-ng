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

#ifndef _FCITX_UTILS_STRING_HASHSET_H_
#define _FCITX_UTILS_STRING_HASHSET_H_

#include "macro.h"
#include "types.h"
#include "dict.h"

FCITX_DECL_BEGIN

typedef FcitxDict FcitxStringHashSet;

static inline FcitxStringHashSet* fcitx_string_hashset_new()
{
    return fcitx_dict_new(NULL);
}

static inline bool fcitx_string_hashset_insert(FcitxStringHashSet* sset, const char* str)
{
    return fcitx_dict_insert_by_str(sset, str, NULL, false);
}

static inline bool fcitx_string_hashset_insert_len(FcitxStringHashSet* sset, const char* str, size_t len)
{
    return fcitx_dict_insert(sset, str, len, NULL, false);
}

static inline bool fcitx_string_hashset_contains(FcitxStringHashSet* sset, const char* str)
{
    return fcitx_dict_lookup_by_str(sset, str, NULL);
}

static inline bool fcitx_string_hashset_remove(FcitxStringHashSet* sset, const char* str)
{
    return fcitx_dict_remove_by_str(sset, str, NULL);
}

static inline void fcitx_string_hashset_free(FcitxStringHashSet* sset)
{
    fcitx_dict_free(sset);
}

static inline FcitxStringHashSet* fcitx_string_hashset_clone(FcitxStringHashSet* sset)
{
    return fcitx_dict_clone(sset, NULL);
}

/**
 * join a string hash set with delimiter
 *
 * @param sset string hash set
 * @param delim delimeter
 * @return char*
 *
 * @since 4.2.7
 **/
char* fcitx_string_hashset_join(FcitxStringHashSet* sset, char delim);

/**
 * parse a string with delimiter
 *
 * @param str string
 * @param delim delimiter
 * @return FcitxStringHashSet*
 *
 * @since 4.2.7
 **/
FcitxStringHashSet* fcitx_string_hashset_parse(const char* str, char delim);

FCITX_DECL_END

#endif
