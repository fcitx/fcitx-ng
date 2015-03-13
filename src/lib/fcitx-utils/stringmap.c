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
#include "stringmap.h"
#include "uthash.h"
#include "stringutils.h"
#include "stringlist.h"

typedef struct _FcitxStringMapItem {
    char* key;
    bool value;
    UT_hash_handle hh;
} FcitxStringMapItem;

struct _FcitxStringMap {
    FcitxStringMapItem* items;
};

static inline void
fcitx_string_map_item_free(FcitxStringMapItem* item)
{
    free(item->key);
    free(item);
}

FCITX_EXPORT_API
FcitxStringMap* fcitx_string_map_new(const char* str, char delim)
{
    FcitxStringMap* map = fcitx_utils_new(FcitxStringMap);
    if (str) {
        fcitx_string_map_from_string(map, str, delim);
    }
    return map;
}

FCITX_EXPORT_API
void fcitx_string_map_from_string(FcitxStringMap* map, const char* str, char delim)
{
    const char delim_string[] = {delim, '\0'};
    const char delim_string_colon[] = {':', '\0'};
    fcitx_string_map_clear(map);
    FcitxStringList* list = fcitx_utils_string_split(str, delim_string);
    utarray_foreach(s, list, char*) {
        FcitxStringList* item = fcitx_utils_string_split(*s, delim_string_colon);
        if (utarray_len(item) == 2) {
            char* key = *(char**) utarray_eltptr(item, 0);
            char* value = *(char**) utarray_eltptr(item, 1);
            bool bvalue = strcmp(value, "true") == 0;
            fcitx_string_map_set(map, key, bvalue);
        }
        fcitx_utils_string_list_free(item);
    }
    fcitx_utils_string_list_free(list);
}

FCITX_EXPORT_API bool
fcitx_string_map_get(FcitxStringMap* map, const char* key, bool _default)
{
    FcitxStringMapItem* item = NULL;
    HASH_FIND_STR(map->items, key, item);
    if (item)
        return item->value;
    return _default;
}

FCITX_EXPORT_API
void fcitx_string_map_set(FcitxStringMap* map, const char* key, bool value)
{
    FcitxStringMapItem* item = NULL;
    HASH_FIND_STR(map->items, key, item);
    if (!item) {
        item = fcitx_utils_new(FcitxStringMapItem);
        item->key = fcitx_utils_strdup(key);
        HASH_ADD_KEYPTR(hh, map->items, item->key, strlen(item->key), item);
    }
    item->value = value;
}

FCITX_EXPORT_API void
fcitx_string_map_clear(FcitxStringMap* map)
{
    while (map->items) {
        FcitxStringMapItem* p = map->items;
        HASH_DEL(map->items, p);
        fcitx_string_map_item_free(p);
    }
}

FCITX_EXPORT_API
void fcitx_string_map_free(FcitxStringMap* map)
{
    fcitx_string_map_clear(map);
    free(map);
}

FCITX_EXPORT_API
void fcitx_string_map_remove(FcitxStringMap* map, const char* key)
{
    FcitxStringMapItem* item = NULL;
    HASH_FIND_STR(map->items, key, item);
    if (item) {
        HASH_DEL(map->items, item);
        fcitx_string_map_item_free(item);
    }
}

FCITX_EXPORT_API
char* fcitx_string_map_to_string(FcitxStringMap* map, char delim)
{
    if (HASH_COUNT(map->items) == 0)
        return fcitx_utils_strdup("");

    size_t len = 0;
    HASH_FOREACH(item, map->items, FcitxStringMapItem) {
        len += item->hh.keylen + 1 + (item->value ? strlen("true") :
                                      strlen("false")) + 1;
    }

    char* result = (char*)fcitx_utils_malloc(sizeof(char) * len);
    char* p = result;
    HASH_FOREACH(item2, map->items, FcitxStringMapItem) {
        size_t strl = item2->hh.keylen;
        memcpy(p, item2->key, strl);
        p += strl;
        *p = ':';
        p++;
        if (item2->value) {
            strl = strlen("true");
            memcpy(p, "true", strlen("true"));
        } else {
            strl = strlen("false");
            memcpy(p, "false", strlen("false"));
        }
        p += strl;
        *p = delim;
        p++;
    }
    result[len - 1] = '\0';

    return result;
}
