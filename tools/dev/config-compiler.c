
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
        typeName = "UT_array";
    }
    return typeName;
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

    fprintf(fout, "struct _%s {\n", fullName);
    fcitx_dict_foreach(structs, print_struct_member, (void*) prefix);
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

bool print_load_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "    %s_load(&data->%s, config);\n", underscoreFullName, name);
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
    if (strcmp(type, "Integer") == 0) {
        fprintf(fout, "    fcitx_configuration_get_integer(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        fprintf(fout, "    fcitx_configuration_get_string(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
    } else if (strcmp(type, "Boolean") == 0) {
        fprintf(fout, "    fcitx_configuration_get_boolean(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
    } else if (strcmp(type, "Char") == 0) {
        fprintf(fout, "    fcitx_configuration_get_char(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
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
            if (strcmp(enumString, defaultValue) == 0) {
                enumDefault = i;
            }
        }
        fprintf(fout, "        };\n");
        fprintf(fout, "        fcitx_configuration_get_enum(config, \"%s\", enumStrings, enumCount, %d, &data->%s);\n", originName, enumDefault, name);
        fprintf(fout, "    } while(0);\n");
    } else if (strcmp(type, "I18NString") == 0) {
        fprintf(fout, "    fcitx_configuration_get_i18n_string(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
    } else if (strcmp(type, "Hotkey") == 0) {
        fprintf(fout, "    fcitx_configuration_get_key(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
    } else if (strcmp(type, "Color") == 0) {
        fprintf(fout, "    fcitx_configuration_get_color(config, \"%s\", \"%s\", &data->%s);\n", originName, defaultValue, name);
    } else if (strcmp(type, "List") == 0) {
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
    if (strcmp(type, "Integer") == 0) {
        fprintf(fout, "    fcitx_configuration_set_integer(config, \"%s\", data->%s);\n", originName, name);
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        fprintf(fout, "    fcitx_configuration_set_string(config, \"%s\", data->%s);\n", originName, name);
    } else if (strcmp(type, "Boolean") == 0) {
        fprintf(fout, "    fcitx_configuration_set_boolean(config, \"%s\", data->%s);\n", originName, name);
    } else if (strcmp(type, "Char") == 0) {
        fprintf(fout, "    fcitx_configuration_set_char(config, \"%s\", data->%s);\n", originName, name);
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
    } else if (strcmp(type, "I18NString") == 0) {
        fprintf(fout, "    fcitx_configuration_set_i18n_string(config, \"%s\", data->%s);\n", originName, name);
    } else if (strcmp(type, "Hotkey") == 0) {
        fprintf(fout, "    fcitx_configuration_set_key(config, \"%s\", data->%s);\n", originName, name);
    } else if (strcmp(type, "Color") == 0) {
        fprintf(fout, "    fcitx_configuration_set_color(config, \"%s\", data->%s);\n", originName, name);
    } else if (strcmp(type, "List") == 0) {
        fprintf(fout, "    fcitx_configuration_set_list(config, \"%s\", data->%s);\n", originName, name);
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
    if (strcmp(type, "Integer") == 0 && strcmp(type, "Boolean") == 0 &&
        strcmp(type, "Char") == 0 && strcmp(type, "Enum") == 0 &&
        strcmp(type, "Color") == 0) {
        return;
    } else if (strcmp(type, "Hotkey") == 0) {
        fprintf(fout, "    fcitx_key_list_free(data->%s);\n", name);
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        fprintf(fout, "    free(data->%s);\n", name);
    } else if (strcmp(type, "I18NString") == 0) {
        fprintf(fout, "    fcitx_i18n_string_free(data->%s);\n", name);
    } else if (strcmp(type, "List") == 0) {
        fprintf(fout, "    utarray_free(data->%s);\n", name);
    }
    free(name);
}

bool print_struct_function_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    fprintf(fout, "void %s_load(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_load_attribute, NULL);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_store(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_store_attribute, NULL);
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
    fcitx_dict_foreach(structs, print_struct_function_definition, &context);

    fprintf(fout, "%2$s* %1$s_new()\n"
                  "{\n"
                  "    return fcitx_utils_new(%2$s);\n"
                  "}\n", underscoreFullName, fullName);
    fprintf(fout, "void %s_load(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(structs, print_load_struct_member, (void*) prefix);
    fprintf(fout, "}\n");
    fprintf(fout, "void %s_free(%s* data)\n", underscoreFullName, fullName);
    fprintf(fout, "{\n");
    fcitx_dict_foreach(structs, print_free_struct_member, (void*) prefix);
    fprintf(fout, "    free(data);\n");
    fprintf(fout, "}\n");
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
