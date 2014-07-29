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
