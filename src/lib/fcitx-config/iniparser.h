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

#ifndef _FCITX_UTILS_INI_PARSER_H_
#define _FCITX_UTILS_INI_PARSER_H_

#include "configuration.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

FcitxConfiguration* fcitx_ini_parse(FILE* fp, FcitxConfiguration* config);

FcitxConfiguration* fcitx_ini_parse_string(const char* str, size_t length, FcitxConfiguration* config);

void fcitx_ini_print(FcitxConfiguration* config, FILE* fp);

char* fcitx_ini_to_string(FcitxConfiguration* config, size_t* pLength);

#endif
