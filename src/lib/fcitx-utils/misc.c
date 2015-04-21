/*
 * Copyright (C) 2012~2015 by CSSlayer
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
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "utils.h"

FCITX_EXPORT_API
void fcitx_utils_closure_free(void* data, void* userData)
{
    FCITXGCLIENT_UNUSED(userData);
    free(data);
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
            result = fcitx_utils_strdup(FCITX_INSTALL_DATADIR);
        }
    }
    else if (strcmp(type, "pkgdatadir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/share/fcitx");
        } else {
            result = fcitx_utils_strdup(FCITX_INSTALL_PKGDATADIR);
        }
    }
    else if (strcmp(type, "bindir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/bin");
        }
        else
            result = fcitx_utils_strdup(FCITX_INSTALL_BINDIR);
    }
    else if (strcmp(type, "libdir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/lib");
        }
        else
            result = fcitx_utils_strdup(FCITX_INSTALL_LIBDIR);
    }
    else if (strcmp(type, "localedir") == 0) {
        if (fcitxdir) {
            fcitx_utils_alloc_cat_str(result, fcitxdir, "/share/locale");
        }
        else
            result = fcitx_utils_strdup(FCITX_INSTALL_LOCALEDIR);
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
    if (*obj == str) {
        return;
    }

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

FCITX_EXPORT_API
size_t
fcitx_utils_read_uint32(FILE *fp, uint32_t *p)
{
    uint32_t res = 0;
    size_t size;
    size = fread(&res, sizeof(uint32_t), 1, fp);
    *p = le32toh(res);
    return size;
}

FCITX_EXPORT_API
size_t
fcitx_utils_write_uint32(FILE *fp, uint32_t i)
{
    i = htole32(i);
    return fwrite(&i, sizeof(uint32_t), 1, fp);
}

FCITX_EXPORT_API
size_t
fcitx_utils_read_uint16(FILE *fp, uint16_t *p)
{
    uint16_t res = 0;
    size_t size;
    size = fread(&res, sizeof(uint16_t), 1, fp);
    *p = le16toh(res);
    return size;
}

FCITX_EXPORT_API
size_t
fcitx_utils_write_uint16(FILE *fp, uint16_t i)
{
    i = htole16(i);
    return fwrite(&i, sizeof(uint16_t), 1, fp);
}

FCITX_EXPORT_API
size_t
fcitx_utils_read_uint64(FILE *fp, uint64_t *p)
{
    uint64_t res = 0;
    size_t size;
    size = fread(&res, sizeof(uint64_t), 1, fp);
    *p = le64toh(res);
    return size;
}

FCITX_EXPORT_API
size_t
fcitx_utils_write_uint64(FILE *fp, uint64_t i)
{
    i = htole64(i);
    return fwrite(&i, sizeof(uint64_t), 1, fp);
}

typedef void (*_fcitx_sighandler_t) (int);

FCITX_EXPORT_API
void fcitx_utils_init_as_daemon()
{
    pid_t pid;
    if ((pid = fork()) > 0) {
        waitpid(pid, NULL, 0);
        exit(0);
    }
    setsid();
    _fcitx_sighandler_t oldint = signal(SIGINT, SIG_IGN);
    _fcitx_sighandler_t oldhup  =signal(SIGHUP, SIG_IGN);
    _fcitx_sighandler_t oldquit = signal(SIGQUIT, SIG_IGN);
    _fcitx_sighandler_t oldpipe = signal(SIGPIPE, SIG_IGN);
    _fcitx_sighandler_t oldttou = signal(SIGTTOU, SIG_IGN);
    _fcitx_sighandler_t oldttin = signal(SIGTTIN, SIG_IGN);
    _fcitx_sighandler_t oldchld = signal(SIGCHLD, SIG_IGN);
    if (fork() > 0)
        exit(0);
    chdir("/");

    signal(SIGINT, oldint);
    signal(SIGHUP, oldhup);
    signal(SIGQUIT, oldquit);
    signal(SIGPIPE, oldpipe);
    signal(SIGTTOU, oldttou);
    signal(SIGTTIN, oldttin);
    signal(SIGCHLD, oldchld);
}
