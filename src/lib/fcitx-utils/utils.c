#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "stringutils.h"
#include "utils.h"

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

FCITX_EXPORT_API
char* fcitx_utils_get_fcitx_path(const char* type)
{
    char* fcitxdir = getenv("FCITXDIR");
    char* result = NULL;
    if (strcmp(type, "datadir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/share");
        } else {
            result = strdup(FCITX_INSTALL_DATADIR);
        }
    }
    else if (strcmp(type, "pkgdatadir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/share/fcitx");
        } else {
            result = strdup(FCITX_INSTALL_PKGDATADIR);
        }
    }
    else if (strcmp(type, "bindir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/bin");
        }
        else
            result = strdup(FCITX_INSTALL_BINDIR);
    }
    else if (strcmp(type, "libdir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/lib");
        }
        else
            result = strdup(FCITX_INSTALL_LIBDIR);
    }
    else if (strcmp(type, "localedir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/share/locale");
        }
        else
            result = strdup(FCITX_INSTALL_LOCALEDIR);
    }
    return result;
}

FCITX_EXPORT_API
char* fcitx_utils_get_fcitx_path_with_filename(const char* type, const char* filename)
{
    char* path = fcitx_utils_get_fcitx_path(type);
    if (path == NULL)
        return NULL;
    char *result;
    fcitx_utils_alloc_cat_str(result, path, "/", filename);
    free(path);
    return result;
}

FCITX_EXPORT_API
void fcitx_utils_string_swap(char** obj, const char* str)
{
    if (str) {
        *obj = fcitx_utils_set_str(*obj, str);
    } else if (*obj) {
        free(*obj);
        *obj = NULL;
    }
}

FCITX_EXPORT_API
void fcitx_utils_string_swap_with_len(char** obj, const char* str, size_t len)
{
    if (str) {
        *obj = fcitx_utils_set_str_with_len(*obj, str, len);
    } else if (*obj) {
        free(*obj);
        *obj = NULL;
    }
}


FCITX_EXPORT_API
void *fcitx_utils_custom_bsearch(const void *key, const void *base,
                                 size_t nmemb, size_t size, int accurate,
                                 int (*compar)(const void *, const void *))
{
    if (accurate)
        return bsearch(key, base, nmemb, size, compar);
    else {
        size_t l, u, idx;
        const void *p;
        int comparison;

        l = 0;
        u = nmemb;
        while (l < u) {
            idx = (l + u) / 2;
            p = (void *)(((const char *) base) + (idx * size));
            comparison = (*compar)(key, p);
            if (comparison <= 0)
                u = idx;
            else if (comparison > 0)
                l = idx + 1;
        }

        if (u >= nmemb)
            return NULL;
        else
            return (void *)(((const char *) base) + (l * size));
    }
}