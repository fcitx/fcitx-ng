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

#include <stdio.h>
#include "common.h"

void remove_invalid_char(char* str)
{
    size_t i = 0, j = 0;
    while(str[i]) {
        if (!fcitx_utils_isspace(str[i])
         && str[i] != '.') {
            if (str[i] == '-') {
                str[j] = '_';
            } else {
                str[j] = str[i];
            }
            j++;
        }
        i++;
    }
    str[j] = 0;
}

char* type_name(const char* prefix, const char* groupName)
{
    char* fullName = NULL;
    fcitx_asprintf(&fullName, "%s%sGroup", prefix, groupName);
    remove_invalid_char(fullName);
    return fullName;
}


char* format_first_lower_name(const char* name)
{
    char* result = fcitx_utils_strdup(name);
    remove_invalid_char(result);
    size_t i = 0;
    while (fcitx_utils_isupper(result[i])) {
        result[i] = fcitx_utils_tolower(result[i]);
        i ++;
    }

    if (i > 1) {
        i --;
        result[i] = fcitx_utils_toupper(result[i]);
    }

    return result;
}

char* format_underscore_name(const char* _name, bool toupper)
{
    char* name = fcitx_utils_strdup(_name);
    remove_invalid_char(name);
    size_t len = strlen(name);
    char* newName = fcitx_utils_malloc(len * 2 + 1);
    size_t j = 0;
    for (size_t i = 0; i < len; i ++) {
        newName[j] = toupper ? fcitx_utils_toupper(name[i]) : fcitx_utils_tolower(name[i]);
        j++;
        if (fcitx_utils_islower(name[i]) && fcitx_utils_isupper(name[i + 1]) && i != 0) {
            newName[j] = '_';
            j++;
        }
    }
    newName[j] = '\0';
    free(name);
    return newName;

}

const char* get_c_type_name(const char* type)
{
    const char* typeName = NULL;
    if (strcmp(type, "Integer") == 0) {
        typeName = "int";
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        typeName = "char*";
    } else if (strcmp(type, "Boolean") == 0) {
        typeName = "bool";
    } else if (strcmp(type, "Char") == 0) {
        typeName = "char";
    } else if (strcmp(type, "Enum") == 0) {
        typeName = "uint32_t";
    } else if (strcmp(type, "I18NString") == 0) {
        typeName = "FcitxI18NString*";
    } else if (strcmp(type, "Hotkey") == 0) {
        typeName = "FcitxKeyList*";
    } else if (strcmp(type, "Color") == 0) {
        typeName = "FcitxColor";
    } else if (strcmp(type, "List") == 0) {
        typeName = "FcitxPtrArray*";
    }
    return typeName;
}

const char* get_load_func(const char* type)
{
    if (strcmp(type, "Integer") == 0) {
        return "fcitx_configuration_get_integer";
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        return "fcitx_configuration_get_string";
    } else if (strcmp(type, "Boolean") == 0) {
        return "fcitx_configuration_get_boolean";
    } else if (strcmp(type, "Char") == 0) {
        return "fcitx_configuration_get_char";
    } else if (strcmp(type, "I18NString") == 0) {
        return "fcitx_configuration_get_i18n_string";
    } else if (strcmp(type, "Hotkey") == 0) {
        return "fcitx_configuration_get_key";
    } else if (strcmp(type, "Color") == 0) {
        return "fcitx_configuration_get_color";
    } else if (strcmp(type, "Enum") == 0) {
        return "fcitx_configuration_get_enum";
    } else if (strcmp(type, "List") == 0) {
        return "fcitx_configuration_get_list";
    }
    return NULL;
}

const char* get_store_func(const char* type)
{
    if (strcmp(type, "Integer") == 0) {
        return "fcitx_configuration_set_integer";
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        return "fcitx_configuration_set_string";
    } else if (strcmp(type, "Boolean") == 0) {
        return "fcitx_configuration_set_boolean";
    } else if (strcmp(type, "Char") == 0) {
        return "fcitx_configuration_set_char";
    } else if (strcmp(type, "I18NString") == 0) {
        return "fcitx_configuration_set_i18n_string";
    } else if (strcmp(type, "Hotkey") == 0) {
        return "fcitx_configuration_set_key";
    } else if (strcmp(type, "Color") == 0) {
        return "fcitx_configuration_set_color";
    } else if (strcmp(type, "Enum") == 0) {
        return "fcitx_configuration_set_enum";
    } else if (strcmp(type, "List") == 0) {
        return "fcitx_configuration_set_list";
    }
    return NULL;
}

const char* get_free_func(const char* type)
{
    if (strcmp(type, "String") == 0 ||
        strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        return "free";
    } else if (strcmp(type, "I18NString") == 0) {
        return "fcitx_i18n_string_free";
    } else if (strcmp(type, "Hotkey") == 0) {
        return "fcitx_key_list_free";
    } else if (strcmp(type, "List") == 0) {
        return "fcitx_ptr_array_free";
    }
    return NULL;
}

const char* get_list_free_func(const char* type)
{
    if (strcmp(type, "String") == 0 ||
        strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        return "fcitx_configuration_string_free";
    } else if (strcmp(type, "I18NString") == 0) {
        return "fcitx_configuration_i18n_string_free";
    } else if (strcmp(type, "Hotkey") == 0) {
        return "fcitx_configuration_key_list_free";
    }
    return NULL;
}
