#ifndef _FCITX_UTILS_FS_H_
#define _FCITX_UTILS_FS_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stddef.h>
#include "types.h"
#include "macro.h"

FCITX_DECL_BEGIN

bool fcitx_utils_isdir(const char* path);
bool fcitx_utils_isreg(const char *path);
bool fcitx_utils_islnk(const char *path);

size_t fcitx_utils_clean_path(const char* path, char* buf);
bool fcitx_utils_make_path(const char *path);

FCITX_DECL_END

#endif
