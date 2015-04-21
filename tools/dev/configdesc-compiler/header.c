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

#include "main.h"
#include "common.h"

void print_includes(const char* includes)
{
    if (includes) {
        FcitxStringList* includeFiles = fcitx_utils_string_split(includes, ",");
        utarray_foreach(includeFile, includeFiles, char*) {
            fprintf(fout, "#include <%s>\n", *includeFile);
        }
        fcitx_utils_string_list_free(includeFiles);
    }
}

/// print header guard
void print_header_guard(const char* name, const char* prefix)
{
    char* fullName = NULL;
    fcitx_asprintf(&fullName, "%s%s", prefix, name);
    char* uName = format_underscore_name(fullName, true);

    fprintf(fout, "#ifndef _%s_H_\n", uName);
    fprintf(fout, "#define _%s_H_\n", uName);
    fprintf(fout, "#include <fcitx-utils/utils.h>\n");
    fprintf(fout, "#include <fcitx-config/configuration.h>\n");

    free(uName);
    free(fullName);
}

bool print_forward_decl(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(data);
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    fprintf(fout, "typedef struct _%s %s;\n", fullName, fullName);
    free(fullName);
    return false;
}

void print_struct_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITX_UNUSED(path);
    FCITX_UNUSED(userData);
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    const char* typeName = get_c_type_name(type);

    char* name = format_first_lower_name(fcitx_configuration_get_name(config));
    fprintf(fout, "    %s %s;\n", typeName, name);
    free(name);
}

bool print_struct_forward_decl(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(data);
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    fprintf(fout, "struct _%s {\n", fullName);
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_attribute, NULL);
    fprintf(fout, "};\n");
    free(fullName);
    return false;
}

bool print_top_level_struct_attribute(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(data);
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    fprintf(fout, "    %s %s;\n", fullName, name);
    free(name);
    free(fullName);
    return false;
}

void compile_to_c_header(FcitxConfiguration* config, FcitxDescription* desc, const char* name, const char* prefix, const char* includes)
{
    // #ifndef style header guard
    print_header_guard(name, prefix);

    // print extra include files
    print_includes(includes);

    // print top level struct, forward declaration
    char* fullName = NULL;
    fcitx_asprintf(&fullName, "%s%s", prefix, name);
    fprintf(fout, "typedef struct _%s %s;\n", fullName, fullName);
    fcitx_dict_foreach(desc->structs, print_forward_decl, (void*) prefix);

    print_struct_definition_context context;
    context.prefix = prefix;
    context.rootConfig = config;
    fcitx_dict_foreach(desc->structs, print_struct_forward_decl, &context);

    // print top level struct
    fprintf(fout, "struct _%s {\n", fullName);
    fcitx_dict_foreach(desc->topLevelStructs, print_top_level_struct_attribute, (void*) prefix);
    fprintf(fout, "};\n");

    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "%2$s* %1$s_new();\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_load(%s* data, FcitxConfiguration* config);\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_store(%s* data, FcitxConfiguration* config);\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_free(%s* data);\n", underscoreFullName, fullName);
    free(underscoreFullName);

    free(fullName);

    fprintf(fout, "#endif\n");
}
