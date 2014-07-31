
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"

FILE* fout;

typedef enum {
    Operation_PotFile,
    Operation_C_Header,
    Operation_C_Source,
} FcitxConfigDescCompileOperation;

void usage()
{
    fprintf(stderr, "\n");
}

void find_structs_callback(FcitxConfiguration* config,
                          const char* path,
                          void* userData)
{
    if (strcmp(path, "DescriptionFile") == 0) {
        return;
    }
    FcitxDict* structs = userData;
    fcitx_string_hashset_insert(structs, path);
}

FcitxStringHashSet* find_structs(FcitxConfiguration* config)
{
    FcitxStringHashSet* structs = fcitx_string_hashset_new();
    // search one level
    fcitx_configuration_foreach(config, "", false, NULL, find_structs_callback, structs);
    // char *structsString = fcitx_string_hashset_join(structs, ',');
    // fprintf(fout, "%s\n", structsString);

    return structs;
}

typedef struct
{
    FcitxConfiguration* rootConfig;
    FcitxStringHashSet* topLevelStructs;
} find_top_level_structs_context;

void struct_attribute_foreach(FcitxConfiguration* config, const char* path, void* userData)
{
    find_top_level_structs_context *context = userData;
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (strcmp(type, "List") != 0) {
        return;
    }
    const char* subType = fcitx_configuration_get_value_by_path(config, "SubType");

    fcitx_string_hashset_remove(context->topLevelStructs, subType);
}

bool structs_foreach(const char* key, size_t keyLen, void** data, void* userData)
{
    find_top_level_structs_context *context = userData;
    fcitx_configuration_foreach(context->rootConfig, key, false, "", struct_attribute_foreach, userData);
    return false;
}

FcitxStringHashSet* find_top_level_structs(FcitxConfiguration* config, FcitxStringHashSet* structs)
{
    FcitxStringHashSet* topLevelStructs = fcitx_string_hashset_clone(structs);
    fcitx_configuration_foreach(config, "", false, NULL, find_structs_callback, structs);

    find_top_level_structs_context context;
    context.rootConfig = config;
    context.topLevelStructs = topLevelStructs;
    fcitx_dict_foreach(structs, structs_foreach, &context);

    return topLevelStructs;
}

char* format_first_lower_name(const char* name)
{
    char* result = strdup(name);
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

char* format_underscore_name(const char* name, bool toupper)
{
    size_t len = strlen(name);
    char* newName = malloc(len * 2 + 1);
    size_t j = 0;
    for (size_t i = 0; i < len; i ++) {
        if (fcitx_utils_isupper(name[i]) && fcitx_utils_islower(name[i + 1]) && i != 0) {
            newName[j] = '_';
            j++;
        }

        newName[j] = toupper ? fcitx_utils_toupper(name[i]) : fcitx_utils_tolower(name[i]);
        j++;
    }
    newName[j] = '\0';
    return newName;
}

void print_includes(const char* includes)
{
    if (includes) {
        FcitxStringList* includeFiles = fcitx_utils_string_split(includes, ",");
        utarray_foreach(includeFile, includeFiles, char*) {
            fprintf(fout, "#include <%s>\n", *includeFile);
        }
    }
}

void print_header_guard(const char* name, const char* prefix)
{
    char* fullName = NULL;
    asprintf(&fullName, "%s%s", prefix, name);
    char* uName = format_underscore_name(fullName, true);

    fprintf(fout, "#ifndef _%s_H_\n", uName);
    fprintf(fout, "#define _%s_H_\n", uName);
    fprintf(fout, "#include <fcitx-utils/utils.h>\n");
    fprintf(fout, "#include <fcitx-config/configuration.h>\n");

    free(uName);
    free(fullName);
}

char* type_name(const char* prefix, const char* groupName)
{
    char* fullName = NULL;
    asprintf(&fullName, "%s%sGroup", prefix, groupName);
    return fullName;
}

bool print_forward_decl(const char* key, size_t keyLen, void** data, void* userData)
{
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    fprintf(fout, "typedef struct _%s %s;\n", fullName, fullName);
    free(fullName);
    return false;
}

typedef struct {
    const char* prefix;
    FcitxConfiguration* rootConfig;
} print_struct_definition_context;

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

void print_struct_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    const char* typeName = get_c_type_name(type);

    char* name = format_first_lower_name(fcitx_configuration_get_name(config));
    fprintf(fout, "    %s %s;\n", typeName, name);
    free(name);
}

bool print_struct_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    fprintf(fout, "struct _%s {\n", fullName);
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_attribute, NULL);
    fprintf(fout, "};\n");
    free(fullName);
    return false;
}

bool print_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    fprintf(fout, "    %s %s;\n", fullName, name);
    free(name);
    free(fullName);
    return false;
}

void compile_to_c_header(FcitxConfiguration* config, FcitxStringHashSet* structs, const char* name, const char* prefix, const char* includes)
{
    print_header_guard(name, prefix);
    print_includes(includes);
    // print top level struct
    char* fullName = NULL;
    asprintf(&fullName, "%s%s", prefix, name);
    fprintf(fout, "typedef struct _%s %s;\n", fullName, fullName);

    fcitx_dict_foreach(structs, print_forward_decl, (void*) prefix);

    print_struct_definition_context context;
    context.prefix = prefix;
    context.rootConfig = config;
    fcitx_dict_foreach(structs, print_struct_definition, &context);

    FcitxStringHashSet* topLevelStructs = find_top_level_structs(config, structs);
    fprintf(fout, "struct _%s {\n", fullName);
    fcitx_dict_foreach(topLevelStructs, print_struct_member, (void*) prefix);
    fprintf(fout, "};\n");
    fcitx_string_hashset_free(topLevelStructs);

    char* underscoreFullName = format_underscore_name(fullName, false);

    fprintf(fout, "%2$s* %1$s_new();\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_load(%s* data, FcitxConfiguration* config);\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_store(%s* data, FcitxConfiguration* config);\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_free(%s* data);\n", underscoreFullName, fullName);

    free(underscoreFullName);
    free(fullName);
    fprintf(fout, "#endif\n");
}

bool print_load_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
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

void print_struct_load_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    const char* originName = fcitx_configuration_get_name(config);
    char* name = format_first_lower_name(originName);
    const char* defaultValue = fcitx_configuration_get_value_by_path(config, "DefaultValue");
    if (defaultValue) {
        fprintf(fout, "    defaultValue = \"%s\";\n", defaultValue);
    } else {
        fprintf(fout, "    defaultValue = NULL;\n");
    }
    if (strcmp(type, "Integer") == 0 ||
        strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0 ||
        strcmp(type, "Char") == 0 ||
        strcmp(type, "I18NString") == 0 ||
        strcmp(type, "Boolean") == 0 ||
        strcmp(type, "Hotkey") == 0 || strcmp(type, "Color") == 0) {
        const char* loadFunc = get_load_func(type);
        fprintf(fout, "    %s(config, \"%s\", defaultValue, &data->%s);\n", loadFunc, originName, name);
    } else if (strcmp(type, "Enum") == 0) {
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
        fprintf(fout, "        fcitx_configuration_get_enum(config, \"%s\", enumStrings, enumCount, %d, &data->%s);\n", originName, enumDefault, name);
        fprintf(fout, "    } while(0);\n");
    } else if (strcmp(type, "List") == 0) {
        const char* subType = fcitx_configuration_get_value_by_path(config, "SubType");
        const char* typeName = get_c_type_name(subType);
        char* structName = NULL;
        char* underscoreName = NULL;
        char* structLoadFunc = NULL;
        char* structFreeFunc = NULL;

        const char* prefix = userData;
        const char* loadFunc;
        const char* freeFunc;
        if (!typeName) {
            structName = type_name(prefix, subType);
            underscoreName = format_underscore_name(structName, false);
            asprintf(&structFreeFunc, "%s_free", underscoreName);
            asprintf(&structLoadFunc, "%s_load", underscoreName);
            typeName = structName;
            freeFunc = structFreeFunc;
            loadFunc = structLoadFunc;
        } else {
            freeFunc = get_free_func(subType);
            loadFunc = get_load_func(subType);
        }

        freeFunc = freeFunc ? freeFunc : "NULL";

        fprintf(fout, "    fcitx_configuration_get_list(config, \"%s\", &data->%s, sizeof(%s), (FcitxConfigurationGetFunc) %s, (FcitxDestroyNotify) %s);\n", originName, name, typeName, loadFunc, freeFunc);
        free(structLoadFunc);
        free(structFreeFunc);
        free(structName);
        free(underscoreName);
    }
    free(name);
}

void print_struct_store_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        return;
    }

    const char* originName = fcitx_configuration_get_name(config);
    char* name = format_first_lower_name(originName);

    if (strcmp(type, "Integer") == 0 ||
        strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0 ||
        strcmp(type, "Char") == 0 ||
        strcmp(type, "I18NString") == 0 ||
        strcmp(type, "Boolean") == 0 ||
        strcmp(type, "Hotkey") == 0 || strcmp(type, "Color") == 0) {
        const char* storeFunc = get_store_func(type);
        fprintf(fout, "    %s(config, \"%s\", &data->%s);\n", storeFunc, originName, name);
    } else if (strcmp(type, "Enum") == 0) {
        fprintf(fout, "    do {\n");
        const char* enumCountString = fcitx_configuration_get_value_by_path(config, "EnumCount");
        int enumCount = atoi(enumCountString);
        fprintf(fout, "        size_t enumCount = %d;\n", enumCount);
        fprintf(fout, "        const char* enumStrings[] = {\n");
        for (int i = 0; i < enumCount; i++) {
            char buf[64];
            sprintf(buf, "Enum%d", i);
            // get Enum0, Enum1.. etc
            const char* enumString = fcitx_configuration_get_value_by_path(config, buf);
            fprintf(fout, "            \"%s\",\n", enumString);
        }
        fprintf(fout, "        };\n");
        fprintf(fout, "        fcitx_configuration_set_enum(config, \"%s\", enumStrings, enumCount, data->%s);\n", originName, name);
        fprintf(fout, "    } while(0);\n");
    } else if (strcmp(type, "List") == 0) {

        const char* subType = fcitx_configuration_get_value_by_path(config, "SubType");
        const char* typeName = get_c_type_name(subType);
        char* structName = NULL;
        char* underscoreName = NULL;
        char* structStoreFunc = NULL;

        const char* prefix = userData;
        const char* storeFunc;
        if (!typeName) {
            structName = type_name(prefix, subType);
            underscoreName = format_underscore_name(structName, false);
            asprintf(&structStoreFunc, "%s_store", underscoreName);
            typeName = structName;
            storeFunc = structStoreFunc;
        } else {
            storeFunc = get_store_func(subType);
        }
        fprintf(fout, "    fcitx_configuration_set_list(config, \"%s\", data->%s, (FcitxConfigurationSetFunc) %s);\n", originName, name, storeFunc);
        free(structStoreFunc);
        free(structName);
        free(underscoreName);
    }
    free(name);
}

void print_struct_free_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
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
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "void %s_load(FcitxConfiguration* config, const char* path, const char* _dummy, %s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fprintf(fout, "    config = fcitx_configuration_get(config, path, false);\n");
    fprintf(fout, "    const char* defaultValue = NULL;\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_load_attribute, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_store(FcitxConfiguration* config, const char* path, %s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fprintf(fout, "    config = fcitx_configuration_get(config, path, true);\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_store_attribute, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_free(%s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_free_attribute, NULL);
    fprintf(fout, "}\n");
    free(underscoreFullName);
    free(fullName);
    return false;
}

void compile_to_c_source(FcitxConfiguration* config, FcitxStringHashSet* structs, const char* name, const char* prefix, const char* includes)
{
    // print top level struct
    char* fullName = NULL;
    asprintf(&fullName, "%s%s", prefix, name);
    char* underscoreFullName = format_underscore_name(fullName, false);

    fprintf(fout, "#include <fcitx-config/helper.h>\n");
    print_includes(includes);

    print_struct_definition_context context;
    context.prefix = prefix;
    context.rootConfig = config;
    fcitx_dict_foreach(structs, print_struct_function_forward_decl, &context);
    fcitx_dict_foreach(structs, print_struct_function_definition, &context);

    FcitxStringHashSet* topLevelStructs = find_top_level_structs(config, structs);

    fprintf(fout, "%2$s* %1$s_new()\n"
                  "{\n"
                  "    %2$s* data = fcitx_utils_new(%2$s);\n"
                  "    %1$s_load(data, NULL);\n"
                  "    return data;\n"
                  "}\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_load(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(topLevelStructs, print_load_struct_member, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_store(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(topLevelStructs, print_store_struct_member, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_free(%s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(topLevelStructs, print_free_struct_member, (void*) prefix);
    fprintf(fout, "    free(data);\n");
    fprintf(fout, "}\n");
    fcitx_string_hashset_free(topLevelStructs);
    free(underscoreFullName);
    free(fullName);

}


void print_pot_header()
{
    fprintf(fout, "# SOME DESCRIPTIVE TITLE.\n"
           "# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER\n"
           "# This file is distributed under the same license as the PACKAGE package.\n"
           "# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n"
           "#\n"
           "#, fuzzy\n"
           "msgid \"\"\n"
           "msgstr \"\"\n"
           "\"Project-Id-Version: PACKAGE VERSION\\n\"\n"
           "\"Report-Msgid-Bugs-To: fcitx-dev@googlegroups.com\\n\"\n"
           "\"POT-Creation-Date: 2014-06-14 15:01+0200\\n\"\n"
           "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
           "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n"
           "\"Language-Team: LANGUAGE <LL@li.org>\\n\"\n"
           "\"Language: LANG\\n\"\n"
           "\"MIME-Version: 1.0\\n\"\n"
           "\"Content-Type: text/plain; charset=utf-8\\n\"\n"
           "\"Content-Transfer-Encoding: 8bit\\n\"\n"
           "\n");
}

void print_pot_string(const char* str)
{
    fprintf(fout, "msgid \"%s\"\n"
           "msgstr \"\"\n"
           "\n", str);
}

typedef struct {
    FcitxConfiguration* rootConfig;
    FcitxStringHashSet* stringSet;
} print_pot_definition_context;

void print_pot_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    FcitxStringHashSet* stringSet = userData;
    fcitx_string_hashset_insert(stringSet, fcitx_configuration_get_name(config));
    const char* description = fcitx_configuration_get_value_by_path(config, "Description");
    if (description) {
        fcitx_string_hashset_insert(stringSet, description);
    }
    const char* LongDescription = fcitx_configuration_get_value_by_path(config, "LongDescription");
    if (LongDescription) {
        fcitx_string_hashset_insert(stringSet, LongDescription);
    }

    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (strcmp(type, "Enum") == 0) {
        const char* enumCountString = fcitx_configuration_get_value_by_path(config, "EnumCount");
        int enumCount = atoi(enumCountString);
        for (int i = 0; i < enumCount; i++) {
            char buf[64];
            sprintf(buf, "Enum%d", i);
            // get Enum0, Enum1.. etc
            const char* enumString = fcitx_configuration_get_value_by_path(config, buf);
            fcitx_string_hashset_insert(stringSet, enumString);
        }
    }
}

bool print_pot_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    print_pot_definition_context *context = userData;
    FcitxConfiguration* config = context->rootConfig;
    fcitx_string_hashset_insert(context->stringSet, key);
    fcitx_configuration_foreach(config, key, false, "", print_pot_attribute, context->stringSet);
    return false;
}

bool print_pot_strings_foreach(const char* key, size_t keyLen, void** data, void* userData)
{
    print_pot_string(key);
    return false;
}

void compile_to_pot(FcitxConfiguration* config, FcitxStringHashSet* structs)
{
    print_pot_header();
    //[Group/Name]
    //Description
    //LongDescription
    FcitxStringHashSet* stringSet = fcitx_string_hashset_new();
    print_pot_definition_context context;
    context.stringSet = stringSet;
    context.rootConfig = config;

    fcitx_dict_foreach(structs, print_pot_definition, &context);

    fcitx_dict_foreach(stringSet, print_pot_strings_foreach, NULL);
    fcitx_string_hashset_free(stringSet);
}

int main(int argc, char* argv[])
{
    int c;
    FcitxConfigDescCompileOperation op = Operation_C_Header;
    char* name = NULL;
    char* prefix = NULL;
    char* output = NULL;
    char* includes = NULL;
    while ((c = getopt(argc, argv, "i:o:n:p:cth")) != EOF) {
        switch (c) {
            case 'i':
                fcitx_utils_string_swap(&includes, optarg);
                break;
            case 'n':
                fcitx_utils_string_swap(&name, optarg);
                break;
            case 'p':
                fcitx_utils_string_swap(&prefix, optarg);
                break;
            case 't':
                op = Operation_PotFile;
                break;
            case 'o':
                fcitx_utils_string_swap(&output, optarg);
                break;
            case 'c':
                op = Operation_C_Source;
                break;
            case 'h':
            default:
                usage();
                exit(0);
                break;
        }
    }

    if (optind >= argc) {
        usage();
        exit(0);
    }

    FcitxConfiguration* config;
    FcitxStringHashSet* structs;
    int result = 0;
    FILE* fin = NULL;

    do {

        fin = fopen(argv[optind], "r");
        if (!fin) {
            perror(NULL);
            result = 1;
            break;
        }

        if (output) {
            fout = fopen(output, "w");
        } else {
            fout = stdout;
        }

        if (!fout) {
            perror(NULL);
            result = 1;
            break;
        }

        config = fcitx_ini_parse(fin, NULL);
        structs = find_structs(config);

        switch(op) {
            case Operation_C_Header:
                if (name) {
                    compile_to_c_header(config, structs, name, prefix ? prefix : "", includes);
                }
                break;
            case Operation_C_Source:
                if (name) {
                    compile_to_c_source(config, structs, name, prefix ? prefix : "", includes);
                }
                break;
            case Operation_PotFile:
                compile_to_pot(config, structs);
                break;
        }
    } while(0);

    if (fin) {
        fclose(fin);
    }
    if (fout) {
        fclose(fout);
    }
    free(output);
    free(prefix);
    free(name);
    fcitx_string_hashset_free(structs);
    fcitx_configuration_unref(config);

    return result;
}
