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
        return strdup("");

    size_t len = 0;
    fcitx_dict_foreach(sset, add_length, &len);

    char* result = (char*)malloc(sizeof(char) * len);
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
