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
void fcitx_color_to_string(const FcitxColor* color, char* str);

#endif // _FCITX_UTILS_COLOR_H_
