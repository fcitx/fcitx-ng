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

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

FCITX_EXPORT_API
bool fcitx_utils_isdir(const char *path)
{
    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode) &&
            access(path, R_OK | X_OK) == 0);
}

FCITX_EXPORT_API
bool fcitx_utils_isreg(const char *path)
{
    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISREG(stats.st_mode) &&
            access(path, R_OK) == 0);
}

FCITX_EXPORT_API
bool fcitx_utils_islnk(const char *path)
{
    struct stat stats;
    return stat(path, &stats) == 0 && S_ISLNK(stats.st_mode);
}

FCITX_EXPORT_API
size_t fcitx_utils_clean_path(const char* path, char* buf)
{
    if (path[0] == '\0') {
        return 0;
    }

    // skip first group of continous slash, for possible furture windows support
    size_t i = 0;
    while (path[i] == '/') {
        buf[i] = path[i];
        i ++;
    }

    size_t j = i;
    int levels = 0;
    while(true) {
        size_t dotcount = 0;
        size_t last = j;
        size_t lasti = i;
        while (path[i] != '\0' && path[i] != '/') {
            if (path[i] == '.') {
                dotcount++;
            }

            buf[j] = path[i];

            i++;
            j++;
        }

        bool eaten = false;
        // everything is a dot
        if (dotcount == i - lasti) {
            if (dotcount == 1) {
                j = last;
                buf[j] = '\0';
                eaten = true;
            } else if (dotcount == 2) {
                if (levels > 0) {
                    for (int k = last - 2; k >= 0; k--) {
                        if (buf[k] == '/') {
                            j = k + 1;
                            buf[j] = '\0';
                            eaten = true;
                            break;
                        }
                    }
                }
            } else {
                levels ++;
            }
        } else {
            levels ++;
        }

        if (path[i] == '\0') {
            buf[j] = '\0';
            break;
        }

        while (path[i] == '/') {
            i ++;
        }

        if (!eaten) {
            buf[j] = '/';
            j++;
        }
    }
    buf[j] = '\0';
    return j;
}

FCITX_EXPORT_API
bool fcitx_utils_make_path(const char *path)
{
    char *p;
    if (fcitx_utils_isdir(path))
        return true;
    size_t len = strlen(path);
    char* opath = fcitx_utils_newv(char, len + 1);
    if (!opath) {
        return false;
    }

    bool result = false;

    len = fcitx_utils_clean_path(path, opath);
    // remove trailling slash
    while (len > 0 && opath[len - 1] == '/') {
        opath[len - 1] = '\0';
        len --;
    }
    if (len == 0) {
        result = true;
        goto make_path_end;
    }


    // skip first /, the root directory or unc is not what we can create
    p = opath;
    while (*p == '/') {
        p++;
    }
    do {
        if (*p == '/' || *p == '\0') {
            char oldchar = *p;
            *p = '\0';

            if (mkdir(opath, S_IRWXU) != 0) {
                if (errno == EEXIST) {
                    struct stat stats;
                    if (stat(opath, &stats) != 0 || !S_ISDIR(stats.st_mode)) {
                        result = false;
                        goto make_path_end;
                    }
                }
            }

            *p = oldchar;
        }
    } while(*(p++) != '\0');

    result = true;

make_path_end:
    free(opath);
    return result;
}
