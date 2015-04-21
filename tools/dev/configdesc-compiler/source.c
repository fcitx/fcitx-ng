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
#include <limits.h>
#include "common.h"
#include "main.h"

bool print_load_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITXGCLIENT_UNUSED(keyLen);
    FCITXGCLIENT_UNUSED(data);
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "    %s_load(config, \"%s\", NULL, &data->%s);\n", underscoreFullName, key, name);
    free(underscoreFullName);
    free(name);
    free(fullName);
    return false;
}

bool print_store_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITXGCLIENT_UNUSED(keyLen);
    FCITXGCLIENT_UNUSED(data);
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "    %s_store(config, \"%s\", &data->%s);\n", underscoreFullName, key, name);
    free(underscoreFullName);
    free(name);
    free(fullName);
    return false;
}

bool print_free_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITXGCLIENT_UNUSED(keyLen);
    FCITXGCLIENT_UNUSED(data);
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "    %s_free(&data->%s);\n", underscoreFullName, name);
    free(underscoreFullName);
    free(name);
    free(fullName);
    return false;
}

void print_option_info(FcitxConfiguration* config, const char* type, const char* prefix)
{
    if (strcmp(type, "Integer") == 0) {
        const char* minStr = fcitx_configuration_get_value_by_path(config, "Min");
        int min = INT_MIN;
        if (minStr) {
            min = atoi(minStr);
        }
        fprintf(fout, "    info.constrain.integer.min  = %d;\n", min);
        const char* maxStr = fcitx_configuration_get_value_by_path(config, "Max");
        int max = INT_MAX;
        if (maxStr) {
            max = atoi(maxStr);
        }
        fprintf(fout, "    info.constrain.integer.max  = %d;\n", max);
    } else if (strcmp(type, "String") == 0) {
        const char* maxLengthStr = fcitx_configuration_get_value_by_path(config, "MaxLength");
        uint32_t maxLength = 0;
        if (maxLengthStr) {
            maxLength = atoi(maxLengthStr);
        }
        fprintf(fout, "    info.constrain.string.maxLength  = %u;\n", maxLength);
    } else if (strcmp(type, "Hotkey") == 0) {
        bool allowModifierOnly = fcitx_utils_strcmp0(fcitx_configuration_get_value_by_path(config, "AllowModifierOnly"), "True") == 0;
        bool disallowNoModifer = fcitx_utils_strcmp0(fcitx_configuration_get_value_by_path(config, "DisallowNoModifer"), "True") == 0;
        fprintf(fout, "    info.constrain.hotkey.allowModifierOnly = %s;\n", allowModifierOnly ? "true" : "false");
        fprintf(fout, "    info.constrain.hotkey.disallowNoModifer = %s;\n", disallowNoModifer ? "true" : "false");
    }

    if (strcmp(type, "Integer") == 0 ||
        strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0 ||
        strcmp(type, "Char") == 0 ||
        strcmp(type, "I18NString") == 0 ||
        strcmp(type, "Boolean") == 0 ||
        strcmp(type, "Hotkey") == 0 || strcmp(type, "Color") == 0) {
        const char* defaultValue = fcitx_configuration_get_value_by_path(config, "DefaultValue");
        if (defaultValue) {
            fprintf(fout, "    info.regular.defaultValue = \"%s\";\n", defaultValue);
        } else {
            fprintf(fout, "    info.regular.defaultValue = NULL;\n");
        }
    } else if (strcmp(type, "Enum") == 0) {
        const char* defaultValue = fcitx_configuration_get_value_by_path(config, "DefaultValue");
        fprintf(fout, "    do {\n");
        const char* enumCountString = fcitx_configuration_get_value_by_path(config, "EnumCount");
        int enumCount = atoi(enumCountString);
        fprintf(fout, "        size_t enumCount = %d;\n", enumCount);
        fprintf(fout, "        const char* enumStrings[] = {\n");
        int enumDefault = 0;
        for (int i = 0; i < enumCount; i++) {
            char buf[64];
            sprintf(buf, "Enum%d", i);
            // get Enum0, Enum1.. etc
            const char* enumString = fcitx_configuration_get_value_by_path(config, buf);
            fprintf(fout, "            \"%s\",\n", enumString);
            if (defaultValue && strcmp(enumString, defaultValue) == 0) {
                enumDefault = i;
            }
        }
        fprintf(fout, "        };\n");
        fprintf(fout, "        info.enumeration.enumCount = enumCount;\n");
        fprintf(fout, "        info.enumeration.enumStrings = enumStrings;\n");
        fprintf(fout, "        info.enumeration.defaultValue = %d;\n", enumDefault);
        fprintf(fout, "    } while(0);\n");
    } else if (strcmp(type, "List") == 0) {
        const char* subType = fcitx_configuration_get_value_by_path(config, "SubType");
        const char* typeName = get_c_type_name(subType);
        char* structName = NULL;
        char* underscoreName = NULL;
        char* structLoadFunc = NULL;
        char* structStoreFunc = NULL;
        char* structFreeFunc = NULL;

        const char* loadFunc;
        const char* storeFunc;
        const char* freeFunc;
        if (!typeName) {
            structName = type_name(prefix, subType);
            underscoreName = format_underscore_name(structName, false);
            fcitx_asprintf(&structFreeFunc, "%s_free", underscoreName);
            fcitx_asprintf(&structLoadFunc, "%s_load", underscoreName);
            fcitx_asprintf(&structStoreFunc, "%s_store", underscoreName);
            typeName = structName;
            freeFunc = structFreeFunc;
            storeFunc = structStoreFunc;
            loadFunc = structLoadFunc;
        } else {
            freeFunc = get_list_free_func(subType);
            loadFunc = get_load_func(subType);
            storeFunc = get_store_func(subType);
        }

        freeFunc = freeFunc ? freeFunc : "NULL";

        fprintf(fout, "    info.list.size = sizeof(%s);\n"
                      "    info.list.loadFunc = (FcitxConfigurationGetFunc) %s;\n"
                      "    info.list.storeFunc = (FcitxConfigurationSetFunc) %s;\n"
                      "    info.list.freeFunc = (FcitxDestroyNotify) %s;\n", typeName, loadFunc, storeFunc, freeFunc);
        print_option_info(config, subType, prefix);
        free(underscoreName);
        free(structLoadFunc);
        free(structFreeFunc);
        free(structName);
    }
}

void print_struct_load_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITXGCLIENT_UNUSED(path);
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    print_option_info(config, type, userData);

    const char* originName = fcitx_configuration_get_name(config);
    char* name = format_first_lower_name(originName);
    const char* loadFunc = get_load_func(type);
    fprintf(fout, "    %s(config, \"%s\", &info, &data->%s);\n", loadFunc, originName, name);
    free(name);
}

void print_struct_store_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITXGCLIENT_UNUSED(path);
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    print_option_info(config, type, userData);

    const char* originName = fcitx_configuration_get_name(config);
    char* name = format_first_lower_name(originName);
    const char* storeFunc = get_store_func(type);
    fprintf(fout, "    %s(config, \"%s\", &info, &data->%s);\n", storeFunc, originName, name);
    free(name);
}

void print_struct_free_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITXGCLIENT_UNUSED(path);
    FCITXGCLIENT_UNUSED(userData);
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    char* name = format_first_lower_name(fcitx_configuration_get_name(config));
    const char* func = get_free_func(type);
    if (func) {
        fprintf(fout, "    %s(data->%s);\n", func, name);
    }
    free(name);
}

bool print_struct_function_forward_decl(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITXGCLIENT_UNUSED(keyLen);
    FCITXGCLIENT_UNUSED(data);
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "void %s_load(FcitxConfiguration* config, const char* path, const char* _dummy, %s* data);\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_store(FcitxConfiguration* config, const char* path, %s* data);\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_free(%s* data);\n", underscoreFullName, fullName);
    free(underscoreFullName);
    free(fullName);
    return false;
}

bool print_struct_function_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITXGCLIENT_UNUSED(keyLen);
    FCITXGCLIENT_UNUSED(data);
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "void %s_load(FcitxConfiguration* config, const char* path, const char* _dummy, %s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fprintf(fout, "    FCITXGCLIENT_UNUSED(_dummy);");
    fprintf(fout, "    config = fcitx_configuration_get(config, path, false);\n");
    fprintf(fout, "    FcitxConfigurationOptionInfo info;\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_load_attribute, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_store(FcitxConfiguration* config, const char* path, %s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fprintf(fout, "    config = fcitx_configuration_get(config, path, true);\n");
    fprintf(fout, "    FcitxConfigurationOptionInfo info;\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_store_attribute, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_free(%s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fprintf(fout, "    FCITXGCLIENT_UNUSED(data);\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_free_attribute, NULL);
    fprintf(fout, "}\n");
    free(underscoreFullName);
    free(fullName);
    return false;
}

void compile_to_c_source(FcitxConfiguration* config, FcitxDescription* desc, const char* name, const char* prefix, const char* includes)
{
    // print top level struct
    char* fullName = NULL;
    fcitx_asprintf(&fullName, "%s%s", prefix, name);
    char* underscoreFullName = format_underscore_name(fullName, false);

    fprintf(fout, "#include <fcitx-config/helper.h>\n");
    print_includes(includes);

    print_struct_definition_context context;
    context.prefix = prefix;
    context.rootConfig = config;
    fcitx_dict_foreach(desc->structs, print_struct_function_forward_decl, &context);
    fcitx_dict_foreach(desc->structs, print_struct_function_definition, &context);

    fprintf(fout, "%2$s* %1$s_new()\n"
                  "{\n"
                  "    %2$s* data = fcitx_utils_new(%2$s);\n"
                  "    %1$s_load(data, NULL);\n"
                  "    return data;\n"
                  "}\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_load(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(desc->topLevelStructs, print_load_struct_member, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_store(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(desc->topLevelStructs, print_store_struct_member, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_free(%s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(desc->topLevelStructs, print_free_struct_member, (void*) prefix);
    fprintf(fout, "    free(data);\n");
    fprintf(fout, "}\n");
    free(underscoreFullName);
    free(fullName);
}
