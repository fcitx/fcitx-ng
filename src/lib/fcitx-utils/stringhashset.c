#include <stdlib.h>
#include "utils.h"
#include "stringhashset.h"

FCITX_EXPORT_API
char* fcitx_utils_string_hash_set_join(FcitxStringHashSet* sset, char delim)
{
    if (!sset)
        return NULL;

    if (HASH_COUNT(sset) == 0)
        return strdup("");

    size_t len = 0;
    HASH_FOREACH(string, sset, FcitxStringHashSet) {
        len += strlen(string->str) + 1;
    }

    char* result = (char*)malloc(sizeof(char) * len);
    char* p = result;
    HASH_FOREACH(string2, sset, FcitxStringHashSet) {
        size_t strl = strlen(string2->str);
        memcpy(p, string2->str, strl);
        p += strl;
        *p = delim;
        p++;
    }
    result[len - 1] = '\0';

    return result;
}

FCITX_EXPORT_API
FcitxStringHashSet* fcitx_utils_string_hash_set_parse(const char* str, char delim)
{
    FcitxStringHashSet* sset = NULL;
    const char *src = str;
    const char *pos;
    size_t len;

    char delim_s[2] = {delim, '\0'};
    while ((len = strcspn(src, delim_s)), *(pos = src + len)) {
        sset = fcitx_utils_string_hash_set_insert_len(sset, src, len);
        src = pos + 1;
    }
    if (len)
        sset = fcitx_utils_string_hash_set_insert_len(sset, src, len);
    return sset;
}

FCITX_EXPORT_API
FcitxStringHashSet* fcitx_utils_string_hash_set_insert(FcitxStringHashSet* sset, const char* str)
{
    FcitxStringHashSet* string = fcitx_utils_new(FcitxStringHashSet);
    string->str = strdup(str);
    HASH_ADD_KEYPTR(hh, sset, string->str, strlen(string->str), string);
    return sset;
}

FCITX_EXPORT_API
FcitxStringHashSet* fcitx_utils_string_hash_set_insert_len(FcitxStringHashSet* sset, const char* str, size_t len)
{
    FcitxStringHashSet* string = fcitx_utils_new(FcitxStringHashSet);
    string->str = strndup(str, len);
    HASH_ADD_KEYPTR(hh, sset, string->str, strlen(string->str), string);
    return sset;
}

FCITX_EXPORT_API
boolean fcitx_utils_string_hash_set_contains(FcitxStringHashSet* sset, const char* str)
{
    FcitxStringHashSet* string = NULL;
    HASH_FIND_STR(sset, str, string);
    return (string != NULL);
}

FCITX_EXPORT_API
FcitxStringHashSet* fcitx_utils_string_hash_set_remove(FcitxStringHashSet* sset, const char* str)
{
    FcitxStringHashSet* string = NULL;
    HASH_FIND_STR(sset, str, string);
    if (string) {
        HASH_DEL(sset, string);
        free(string->str);
        free(string);
    }
    return sset;
}

FCITX_EXPORT_API
void fcitx_utils_string_hash_set_free(FcitxStringHashSet* sset)
{
    FcitxStringHashSet *curStr;
    while (sset) {
        curStr = sset;
        HASH_DEL(sset, curStr);
        free(curStr->str);
        free(curStr);
    }
}

FCITX_EXPORT_API
int fcitx_utils_string_hash_set_compare(FcitxStringHashSet* sseta, FcitxStringHashSet* ssetb)
{
    return strcmp(sseta->str, ssetb->str);
}
