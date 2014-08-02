/***************************************************************************
 *   Copyright (C) 2013~2013 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "utils.h"
#include "library.h"
#include "stringutils.h"
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

struct _FcitxLibrary
{
    char* path;
    char* errorString;
    void* handle;
};

FCITX_EXPORT_API
FcitxLibrary* fcitx_library_new(const char* path)
{
    FcitxLibrary* lib = fcitx_utils_new(FcitxLibrary);
    if (path) {
        lib->path = strdup(path);
    } else {
        lib->path = NULL;
    }
    return lib;
}

FCITX_EXPORT_API
void fcitx_library_free(FcitxLibrary* lib)
{
    if (!lib) {
        return;
    }
    free(lib->errorString);
    free(lib->path);
    free(lib);
}

FCITX_EXPORT_API
bool fcitx_library_load(FcitxLibrary* lib, uint32_t hint)
{
    int flag = 0;
    if (hint & FLLH_ResolveAllSymbolsHint) {
        flag |= RTLD_NOW;
    } else {
        flag |= RTLD_LAZY;
    }

    if (hint & FLLH_PreventUnloadHint) {
        flag |= RTLD_NODELETE;
    }

    if (hint & FLLH_ExportExternalSymbolsHint) {
        flag |= RTLD_GLOBAL;
    }

    lib->handle = dlopen(lib->path, flag);
    if (!lib->handle) {
        fcitx_utils_string_swap(&lib->errorString, dlerror());
        return false;
    }

    return true;
}

FCITX_EXPORT_API
bool fcitx_library_unload(FcitxLibrary* lib)
{
    if (!lib->handle) {
        return false;
    }

    if (dlclose(lib->handle)) {
        fcitx_utils_string_swap(&lib->errorString, dlerror());
        return false;
    }

    lib->handle = NULL;
    return true;
}

FCITX_EXPORT_API
bool fcitx_library_find_data(FcitxLibrary* lib, const char* slug, const char* magic, size_t lenOfMagic, FcitxLibraryDataParser parser, void* arg)
{
    if (lib->handle) {
        void* data = fcitx_library_resolve(lib, slug);
        if (!data) {
            return false;
        }

        if (memcmp(data, magic, lenOfMagic) != 0) {
            return false;
        }

        data += lenOfMagic;
        if (parser) {
            parser(data, arg);
        }
        return true;
    }

    int fd = open(lib->path, O_RDONLY);
    if (fd < 0) {
        fcitx_utils_string_swap(&lib->errorString, strerror(errno));
        return false;
    }

    void* needfree = NULL;
    do {
        struct stat statbuf;
        int statresult = fstat(fd, &statbuf);
        if (statresult < 0) {
            fcitx_utils_string_swap(&lib->errorString, strerror(errno));
            break;
        }
        void* data = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (!data) {
            data = malloc(statbuf.st_size);
            needfree = data;
            if (!data) {
                break;
            }
            if (read(fd, data, statbuf.st_size) != statbuf.st_size) {
                break;
            }
        }
        char* pos = fcitx_utils_backward_search(data, (size_t) statbuf.st_size, magic, lenOfMagic, 0);
        pos += lenOfMagic;

        if (parser) {
            parser(pos, arg);
        }
        return true;
    } while(0);

    close(fd);

    if (needfree) {
        free(needfree);
    }

    return false;
}

FCITX_EXPORT_API
void* fcitx_library_resolve(FcitxLibrary* lib, const char* symbol)
{
    if (!lib->handle) {
        return NULL;
    }

    void* result = dlsym(lib->handle, symbol);
    if (!result) {
        fcitx_utils_string_swap(&lib->errorString, dlerror());
    }
    return result;
}

FCITX_EXPORT_API
const char* fcitx_library_error(FcitxLibrary* lib)
{
    return lib->errorString;
}

