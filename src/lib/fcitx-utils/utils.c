#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils.h"

FCITX_EXPORT_API const UT_icd *const fcitx_str_icd = &ut_str_icd;
FCITX_EXPORT_API const UT_icd *const fcitx_int_icd = &ut_int_icd;

FCITX_EXPORT_API
void* fcitx_utils_malloc0(size_t bytes)
{
    void *p = malloc(bytes);
    if (!p)
        return NULL;

    memset(p, 0, bytes);
    return p;
}

FCITX_EXPORT_API
void fcitx_utils_free(void* ptr)
{
    free(ptr);
}


FCITX_EXPORT_API FcitxStringList*
fcitx_utils_string_list_new()
{
    FcitxStringList *array;
    utarray_new(array, fcitx_str_icd);
    return array;
}

FCITX_EXPORT_API FcitxStringList*
fcitx_utils_append_split_string(FcitxStringList *list,
                                const char* str, const char *delm)
{
    const char *src = str;
    const char *pos;
    size_t len;
    while ((len = strcspn(src, delm)), *(pos = src + len)) {
        fcitx_utils_string_list_append_len(list, src, len);
        src = pos + 1;
    }
    if (len)
        fcitx_utils_string_list_append_len(list, src, len);
    return list;
}

FCITX_EXPORT_API
FcitxStringList* fcitx_utils_string_split(const char* str, char delm)
{
    FcitxStringList* array;
    char delm_s[2] = {delm, '\0'};
    utarray_new(array, fcitx_str_icd);
    return fcitx_utils_append_split_string(array, str, delm_s);
}

FCITX_EXPORT_API
void fcitx_utils_string_list_printf_append(FcitxStringList* list, const char* fmt,...)
{
    char* buffer;
    va_list ap;
    va_start(ap, fmt);
    vasprintf(&buffer, fmt, ap);
    va_end(ap);
    fcitx_utils_string_list_append_no_copy(list, buffer);
}

FCITX_EXPORT_API
char* fcitx_utils_string_list_join(FcitxStringList* list, char delm)
{
    if (!list)
        return NULL;

    if (utarray_len(list) == 0)
        return strdup("");

    size_t len = 0;
    char** str;
    for (str = (char**) utarray_front(list);
         str != NULL;
         str = (char**) utarray_next(list, str))
    {
        len += strlen(*str) + 1;
    }

    char* result = (char*)malloc(sizeof(char) * len);
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

FCITX_EXPORT_API char*
fcitx_utils_set_str_with_len(char *res, const char *str, size_t len)
{
    if (res) {
        res = realloc(res, len + 1);
    } else {
        res = malloc(len + 1);
    }
    memcpy(res, str, len);
    res[len] = '\0';
    return res;
}
