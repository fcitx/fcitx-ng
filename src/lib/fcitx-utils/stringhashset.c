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

#include <stdlib.h>
#include "utils.h"
#include "stringhashset.h"

bool add_length(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(key);
    FCITX_UNUSED(data);
    size_t* len = userData;
    *len += keyLen + 1;
    return false;
}

typedef struct {
    char* p;
    char delim;
} copy_string_context;

bool copy_string(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(data);
    copy_string_context* c = userData;
    memcpy(c->p, key, keyLen);
    c->p += keyLen;
    *c->p = c->delim;
    c->p++;
    return false;
}

FCITX_EXPORT_API
char* fcitx_string_hashset_join(FcitxStringHashSet* sset, char delim)
{
    if (!sset)
        return NULL;

    if (fcitx_dict_size(sset) == 0)
        return fcitx_utils_strdup("");

    size_t len = 0;
    fcitx_dict_foreach(sset, add_length, &len);

    char* result = (char*)fcitx_utils_malloc(sizeof(char) * len);
    copy_string_context c;
    c.p = result;
    c.delim = delim;
    fcitx_dict_foreach(sset, copy_string, &c);
    result[len - 1] = '\0';

    return result;
}

FCITX_EXPORT_API
FcitxStringHashSet* fcitx_string_hashset_parse(const char* str, char delim)
{
    FcitxStringHashSet* sset = fcitx_string_hashset_new();
    const char *src = str;
    const char *pos;
    size_t len;

    char delim_s[2] = {delim, '\0'};
    while ((len = strcspn(src, delim_s)), *(pos = src + len)) {
        fcitx_string_hashset_insert_len(sset, src, len);
        src = pos + 1;
    }
    if (len) {
        fcitx_string_hashset_insert_len(sset, src, len);
    }
    return sset;
}
