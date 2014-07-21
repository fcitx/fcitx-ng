#ifndef _FCITX_UTILS_STRINGMAP_H_
#define _FCITX_UTILS_STRINGMAP_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include "types.h"
#include "macro.h"

FCITX_DECL_BEGIN

/**
 * FcitxStringMap is a string-to-bool hash table, which mean to easily store/parse
 * a single/multi-line string. like a:True,b:False
 *
 * This can be used in configuration storage.
 */
typedef struct _FcitxStringMap FcitxStringMap;

FcitxStringMap* fcitx_string_map_new(const char* str, char delim);

void fcitx_string_map_from_string(FcitxStringMap* map, const char* str, char delim);

bool fcitx_string_map_get(FcitxStringMap *map, const char *key,
                             bool _default);

void fcitx_string_map_set(FcitxStringMap* map, const char* key, bool value);

void fcitx_string_map_clear(FcitxStringMap* map);

char* fcitx_string_map_to_string(FcitxStringMap* map, char delim);

void fcitx_string_map_remove(FcitxStringMap* map, const char* key);

void fcitx_string_map_free(FcitxStringMap* map);

FCITX_DECL_END

#endif // FCITX_UTILS_STRINGMAP_H
