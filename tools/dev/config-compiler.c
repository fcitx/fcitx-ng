
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"

typedef enum {
    Operation_PotFile,
    Operation_C_Header,
    Operation_C_Source,
} FcitxConfigDescCompileOperation;

void usage()
{
    printf("\n");
}

void find_structs_callback(FcitxConfiguration* config,
                          const char* path,
                          void* userData)
{
    if (strcmp(path, "DescriptionFile") == 0) {
        return;
    }
    FcitxDict* structs = userData;
    fcitx_utils_string_hash_set_insert(structs, path);
}

FcitxStringHashSet* find_structs(FcitxConfiguration* config)
{
    FcitxStringHashSet* structs = fcitx_utils_string_hash_set_new();
    // search one level
    fcitx_configuration_foreach(config, "", false, NULL, find_structs_callback, structs);
    // char *structsString = fcitx_utils_string_hash_set_join(structs, ',');
    // printf("%s\n", structsString);

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

void print_header_guard(const char* name, const char* prefix)
{
    char* fullName = NULL;
    asprintf(&fullName, "%s%s", prefix, name);
    char* uName = format_underscore_name(fullName, true);

    printf("#ifndef _%s_H_\n", uName);
    printf("#define _%s_H_\n", uName);

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
    printf("typedef struct _%s %s;\n", fullName, fullName);
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
    } else if (strcmp(type, "Array") == 0) {
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
    printf("    %s %s;\n", typeName, name);
    free(name);
}

bool print_struct_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    printf("struct _%s {\n", fullName);
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_attribute, NULL);
    printf("};\n");
    free(fullName);
    return false;
}

bool print_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    printf("    %s %s;\n", fullName, name);
    free(name);
    free(fullName);
    return false;
}

void compile_to_c_header(FcitxConfiguration* config, FcitxStringHashSet* structs, const char* name, const char* prefix)
{
    print_header_guard(name, prefix);
    // print top level struct
    char* fullName = NULL;
    asprintf(&fullName, "%s%s", prefix, name);
    printf("typedef struct _%s %s;\n", fullName, fullName);

    fcitx_dict_foreach(structs, print_forward_decl, (void*) prefix);

    print_struct_definition_context context;
    context.prefix = prefix;
    context.rootConfig = config;
    fcitx_dict_foreach(structs, print_struct_definition, &context);

    printf("struct _%s {\n", fullName);
    fcitx_dict_foreach(structs, print_struct_member, (void*) prefix);
    printf("};\n");

    char* underscoreFullName = format_underscore_name(fullName, false);

    printf("#define %s_new() fcitx_utils_new(%s)\n", underscoreFullName, fullName);
    printf("void %s_load(%s* data, FcitxConfiguration* config);\n", underscoreFullName, fullName);
    printf("void %s_free(%s* data);\n", underscoreFullName, fullName);

    free(underscoreFullName);
    free(fullName);
    printf("#endif\n");
}

bool print_load_struct_member(const char* key, size_t keyLen, void** data, void* userData)
{
    const char* prefix = userData;
    char* fullName = type_name(prefix, key);
    char* name = format_first_lower_name(key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    printf("    %s_load(&data->%s, config);\n", underscoreFullName, name);
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
    printf("    %s_free(&data->%s);\n", underscoreFullName, name);
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

    char* name = format_first_lower_name(fcitx_configuration_get_name(config));
    printf("    if ((str = fcitx_configuration_get_value_by_path(config, \"%s\")) == NULL) { str = \"%s\"; }\n",
           fcitx_configuration_get_name(config),
           fcitx_configuration_get_value_by_path(config, "DefaultValue"));
    if (strcmp(type, "Integer") == 0) {
        printf("    data->%s = atoi(str);\n", name);
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        printf("    fcitx_utils_string_swap(&data->%s, str);\n", name);
    } else if (strcmp(type, "Boolean") == 0) {
        printf("    data->%s = strcmp(str, \"True\") == 0;\n", name);
    } else if (strcmp(type, "Char") == 0) {
        printf("    data->%s = str[0];\n", name);
    } else if (strcmp(type, "Enum") == 0) {
    } else if (strcmp(type, "I18NString") == 0) {
    } else if (strcmp(type, "Hotkey") == 0) {
    } else if (strcmp(type, "Color") == 0) {
    } else if (strcmp(type, "Array") == 0) {
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
        printf("    FcitxKeyListFree(data->%s);\n", name);
    } else if (strcmp(type, "String") == 0 || strcmp(type, "File") == 0 || strcmp(type, "Font") == 0) {
        printf("    free(data->%s);\n", name);
    } else if (strcmp(type, "I18NString") == 0) {
        printf("    fcitx_i18n_string_free(data->%s);\n", name);
    } else if (strcmp(type, "Array") == 0) {
        printf("    utarray_free(data->%s);\n", name);
    }
    free(name);
}

bool print_struct_function_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    print_struct_definition_context* context = userData;
    const char* prefix = context->prefix;
    char* fullName = type_name(prefix, key);
    char* underscoreFullName = format_underscore_name(fullName, false);
    printf("void %s_load(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    printf("{\n");
    printf("    const char* str;\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_load_attribute, NULL);
    printf("}\n");
    printf("void %s_free(%s* data)\n", underscoreFullName, fullName);
    printf("{\n");
    fcitx_configuration_foreach(context->rootConfig, key, false, "", print_struct_free_attribute, NULL);
    printf("}\n");
    free(underscoreFullName);
    free(fullName);
    return false;
}

void compile_to_c_source(FcitxConfiguration* config, FcitxStringHashSet* structs, const char* name, const char* prefix)
{
    // print top level struct
    char* fullName = NULL;
    asprintf(&fullName, "%s%s", prefix, name);
    char* underscoreFullName = format_underscore_name(fullName, false);

    print_struct_definition_context context;
    context.prefix = prefix;
    context.rootConfig = config;
    fcitx_dict_foreach(structs, print_struct_function_definition, &context);

    printf("void %s_load(%s* data, FcitxConfiguration* config)\n", underscoreFullName, fullName);
    printf("{\n");
    fcitx_dict_foreach(structs, print_load_struct_member, (void*) prefix);
    printf("}\n");
    printf("void %s_free(%s* data);\n", underscoreFullName, fullName);
    printf("{\n");
    fcitx_dict_foreach(structs, print_free_struct_member, (void*) prefix);
    printf("    free(data)\n");
    printf("}\n");
    free(underscoreFullName);
    free(fullName);

}

void compile_to_pot(FcitxConfiguration* config, FcitxStringHashSet* structs)
{
}

int main(int argc, char* argv[])
{
    int c;
    FcitxConfigDescCompileOperation op = Operation_C_Header;
    char* name = NULL;
    char* prefix = NULL;
    while ((c = getopt(argc, argv, "n:p:cth")) != EOF) {
        switch (c) {
            case 'n':
                fcitx_utils_string_swap(&name, optarg);
                break;
            case 'p':
                fcitx_utils_string_swap(&prefix, optarg);
                break;
            case 't':
                op = Operation_PotFile;
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

    FILE* fp = fopen(argv[optind], "r");
    if (!fp) {
        perror(NULL);
        return 1;
    }

    FcitxConfiguration* config = fcitx_ini_parse(fp, NULL);
    FcitxStringHashSet* structs = find_structs(config);

    switch(op) {
        case Operation_C_Header:
            if (name) {
                compile_to_c_header(config, structs, name, prefix ? prefix : "");
            }
            break;
        case Operation_C_Source:
            if (name) {
                compile_to_c_source(config, structs, name, prefix ? prefix : "");
            }
            break;
        case Operation_PotFile:
            compile_to_pot(config, structs);
            break;
    }

    free(prefix);
    free(name);
    fcitx_utils_string_hash_set_free(structs);
    fcitx_configuration_unref(config);

    return 0;
}
