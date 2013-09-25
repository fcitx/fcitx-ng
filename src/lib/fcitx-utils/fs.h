#ifndef _FCITX_UTILS_FS_H_
#define _FCITX_UTILS_FS_H_

#include <stddef.h>
#include <fcitx-utils/types.h>
#include <fcitx-utils/macro.h>

FCITX_DECL_BEGIN

boolean fcitx_utils_isdir(const char* path);
boolean fcitx_utils_isreg(const char *path);
boolean fcitx_utils_islnk(const char *path);

size_t fcitx_utils_clean_path(const char* path, char* buf);
boolean fcitx_utils_make_path(const char *path);

FCITX_DECL_END

#endif
