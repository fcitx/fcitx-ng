#ifndef _FCITX_UTILS_COLOR_H_
#define _FCITX_UTILS_COLOR_H_

#include <stdbool.h>

typedef struct _FcitxColor
{
    double red;
    double green;
    double blue;
    double alpha;
} FcitxColor;

// include NUL character
#define FCITX_COLOR_STRING_LENGTH 10

bool fcitx_color_parse(FcitxColor* color, const char* str);
void fcitx_color_to_string(FcitxColor* color, char* str);

#endif // _FCITX_UTILS_COLOR_H_
