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

#ifndef _FCITX_TOOLS_CONFIGDESC_COMPILER_COMMON_H_
#define _FCITX_TOOLS_CONFIGDESC_COMPILER_COMMON_H_

#include <stdbool.h>
#include <fcitx-config/configuration.h>

typedef struct {
    const char* prefix;
    FcitxConfiguration* rootConfig;
} print_struct_definition_context;

char* type_name(const char* prefix, const char* groupName);

char* format_first_lower_name(const char* name);

char* format_underscore_name(const char* name, bool toupper);

const char* get_c_type_name(const char* type);

const char* get_load_func(const char* type);

const char* get_store_func(const char* type);

const char* get_free_func(const char* type);

const char* get_list_free_func(const char* type);

#endif // _FCITX_TOOLS_CONFIGDESC_COMPILER_COMMON_H_
