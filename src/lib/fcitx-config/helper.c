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

#include <assert.h>
#include "helper.h"

static
const char* fcitx_configuration_get_value_by_path_with_default(FcitxConfiguration* config, const char* path, const char* defaultValueUsr, const char* defaultValueSys)
{
    assert(defaultValueSys != NULL);
    const char* value = fcitx_configuration_get_value_by_path(config, path);
    if (value == NULL) {
        value = defaultValueUsr ? defaultValueUsr : defaultValueSys;
    }
    return value;
}

FCITX_EXPORT_API
void fcitx_configuration_get_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, char** str)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "");
    if (info->constrain.string.maxLength && strlen(value) > info->constrain.string.maxLength) {
        value = fcitx_configuration_get_value_by_path_with_default(NULL, path, info->regular.defaultValue, "");
    }
    fcitx_utils_string_swap(str, value);
}

FCITX_EXPORT_API
void fcitx_configuration_set_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, char** str)
{
    FCITX_UNUSED(info);
    if (!*str) {
        return;
    }
    fcitx_configuration_set_value_by_path(config, path, *str);
}

FCITX_EXPORT_API
void fcitx_configuration_get_integer(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, int* integer)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "0");
    int integerValue = atoi(value);
    if (integerValue < info->constrain.integer.min || integerValue > info->constrain.integer.max) {
        integerValue = atoi(fcitx_configuration_get_value_by_path_with_default(NULL, path, info->regular.defaultValue, "0"));
    }

    *integer = integerValue;
}

FCITX_EXPORT_API
void fcitx_configuration_set_integer(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const int* integer)
{
    FCITX_UNUSED(info);
    char buf[64];
    sprintf(buf, "%d", *integer);
    fcitx_configuration_set_value_by_path(config, path, buf);
}

FCITX_EXPORT_API
void fcitx_configuration_get_boolean(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, bool* b)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "False");
    *b = (strcmp(value, "True") == 0);
}

FCITX_EXPORT_API
void fcitx_configuration_set_boolean(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const bool* b)
{
    FCITX_UNUSED(info);
    fcitx_configuration_set_value_by_path(config, path, *b ? "True" : "False");
}

FCITX_EXPORT_API
void fcitx_configuration_get_char(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, char* chr)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "");
    *chr = value[0];
}

FCITX_EXPORT_API
void fcitx_configuration_set_char(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const char* chr)
{
    FCITX_UNUSED(info);
    char buf[2] = {*chr, '\0'};
    fcitx_configuration_set_value_by_path(config, path, buf);
}

FCITX_EXPORT_API
void fcitx_configuration_get_color(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxColor* color)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "#000000ff");
    if (!fcitx_color_parse(color, value)) {
        if (!fcitx_color_parse(color, info->regular.defaultValue)) {
            fcitx_color_parse(color, "#000000ff");
        }
    }
}

FCITX_EXPORT_API
void fcitx_configuration_set_color(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const FcitxColor* color)
{
    FCITX_UNUSED(info);
    char buf[FCITX_COLOR_STRING_LENGTH];
    fcitx_color_to_string(color, buf);
    fcitx_configuration_set_value_by_path(config, path, buf);
}

FCITX_EXPORT_API
void fcitx_configuration_get_key(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxKeyList** keyList)
{
    if (!(*keyList)) {
        *keyList = fcitx_key_list_new();
    }
    fcitx_key_list_clear(*keyList);

    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "");
    fcitx_key_list_parse(*keyList, value);
}

FCITX_EXPORT_API
void fcitx_configuration_set_key(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxKeyList** keyList)
{
    FCITX_UNUSED(info);
    if (!*keyList) {
        return;
    }
    char* buf = fcitx_key_list_to_string(*keyList);
    fcitx_configuration_set_value_by_path(config, path, buf);
    free(buf);
}

typedef struct {
    FcitxI18NString* str;
    const char* prefix;
    size_t prefixLen;
} get_i18n_string_foreach_context;

void get_i18n_string_foreach(FcitxConfiguration* config, const char* path, void* userData)
{
    const char* value;
    if ((value = fcitx_configuration_get_value(config)) == NULL) {
        return;
    }

    get_i18n_string_foreach_context* context = userData;
    if (!fcitx_utils_string_starts_with(path, context->prefix)) {
        return;
    }

    if (path[context->prefixLen] != '[') {
        return;
    }

    size_t len = strlen(path);

    if (path[len - 1] != ']') {
        return;
    }

    fcitx_dict_insert(context->str,
                      &path[context->prefixLen + 1],
                      len - context->prefixLen - 2,
                      fcitx_utils_strdup(fcitx_configuration_get_value(config)),
                      true);
}

FCITX_EXPORT_API
void fcitx_configuration_get_i18n_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxI18NString** pStr)
{
    if (!*pStr) {
        *pStr = fcitx_i18n_string_new();
    }
    FcitxI18NString* str = *pStr;
    fcitx_dict_remove_all(str);
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, info->regular.defaultValue, "");
    fcitx_dict_insert_by_str(str, "", fcitx_utils_strdup(value), false);

    get_i18n_string_foreach_context context;
    context.str = str;
    context.prefix = path;
    context.prefixLen = strlen(path);
    fcitx_configuration_foreach(config, "", false, "", get_i18n_string_foreach, &context);
}

typedef struct {

    FcitxConfiguration* config;
    const char* path;
} set_i18n_string_foreach_context;

bool set_i18n_string_foreach(const char* key, size_t keyLen, void** data, void* arg)
{
    set_i18n_string_foreach_context* context = arg;
    if (keyLen) {
        char* path = NULL;
        fcitx_asprintf(&path, "%s[%s]", context->path, key);
        fcitx_configuration_set_value_by_path(context->config, path, (char*) *data);
        free(path);
    } else {
        fcitx_configuration_set_value_by_path(context->config, context->path, (char*) *data);
    }
    return false;
}

FCITX_EXPORT_API
void fcitx_configuration_set_i18n_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxI18NString** str)
{
    FCITX_UNUSED(info);
    if (!*str) {
        return;
    }

    set_i18n_string_foreach_context context;
    context.config = config;
    context.path = path;
    fcitx_dict_foreach(*str, set_i18n_string_foreach, &context);
}

FCITX_EXPORT_API
void fcitx_configuration_get_enum(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, uint32_t* enumValue)
{
    const char* value = fcitx_configuration_get_value_by_path(config, path);
    if (value == NULL) {
        *enumValue = info->enumeration.defaultValue;
        return;
    }
    size_t i = 0;
    for (i = 0; i < info->enumeration.enumCount; i++) {
        if (strcmp(value, info->enumeration.enumStrings[i]) == 0) {
            break;
        }
    }
    *enumValue = (i == info->enumeration.enumCount) ? info->enumeration.defaultValue : i;
}

FCITX_EXPORT_API
void fcitx_configuration_set_enum(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, uint32_t* enumValue)
{
    if (*enumValue >= info->enumeration.enumCount) {
        return;
    }

    fcitx_configuration_set_value_by_path(config, path, info->enumeration.enumStrings[*enumValue]);
}

void _fcitx_configuration_list_free(void* data, void* userData)
{
    FcitxDestroyNotify freeFunc = userData;
    if (freeFunc) {
        freeFunc(data);
    }
    free(data);
}

FCITX_EXPORT_API
void fcitx_configuration_get_list(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxPtrArray** list)
{
    if (!*list) {
        *list = fcitx_ptr_array_new_full(NULL, NULL, _fcitx_configuration_list_free, info->list.freeFunc);
    }

    fcitx_ptr_array_clear(*list);

    config = fcitx_configuration_get(config, path, false);
    if (!config) {
        return;
    }

    uint32_t i = 0;
    char buf[64];
    while (true) {
        sprintf(buf, "%u", i);
        FcitxConfiguration* subConfig = fcitx_configuration_get(config, buf, false);
        if (!subConfig) {
            break;
        }

        void* data = fcitx_utils_malloc0(info->list.size);
        info->list.loadFunc(subConfig, "", info, data);
        fcitx_ptr_array_append(*list, data);
        i++;
    }

}

FCITX_EXPORT_API
void fcitx_configuration_set_list(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxPtrArray** _list)
{
    FcitxPtrArray* list = *_list;
    config = fcitx_configuration_get(config, path, true);
    // 64 is enough for integer
    char buf[64];
    for (uint32_t i = 0; i < list->len; i++) {
        sprintf(buf, "%u", i);
        void* data = fcitx_ptr_array_index(list, i, void*);
        FcitxConfiguration* subConfig = fcitx_configuration_get(config, buf, true);
        info->list.storeFunc(subConfig, "", info, data);
    }
}
