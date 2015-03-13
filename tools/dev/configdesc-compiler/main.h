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

#ifndef _FCITX_TOOLS_CONFIGDESC_COMPILER_MAIN_H_
#define _FCITX_TOOLS_CONFIGDESC_COMPILER_MAIN_H_

#include "fcitx-utils/utils.h"
#include "fcitx-config/configuration.h"
#include "fcitx-config/description.h"

extern FILE* fout;

void compile_to_pot(FcitxConfiguration* config, FcitxDescription* desc);

void compile_to_c_source(FcitxConfiguration* config, FcitxDescription* desc, const char* name, const char* prefix, const char* includes);

void compile_to_c_header(FcitxConfiguration* config, FcitxDescription* desc, const char* name, const char* prefix, const char* includes);

void print_includes(const char* includes);

bool validate(FcitxConfiguration* config, FcitxStringHashSet* structs);

#endif // _FCITX_TOOLS_CONFIGDESC_COMPILER_MAIN_H_
