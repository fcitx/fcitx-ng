#include <stdlib.h>
#include <string.h>

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

