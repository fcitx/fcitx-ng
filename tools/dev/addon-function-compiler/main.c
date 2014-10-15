#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <gettext-po.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include "common.h"
#include "fxaddon.h"
#include "fxfunction.h"

FILE* fout = NULL;

typedef enum {
    Operation_C_Header,
    Operation_C_Source,
    Operation_C_Header_Internal,
} FcitxAddonFunctionCompileOperation;

char get_sigchar_from_string(const char* type);

void print_c_header(FcitxFunctionInfo* functionInfo, FcitxConfiguration* config)
{
    // print header guard

    char* uName = format_underscore_name(functionInfo->fcitxAddon.name, true);

    fprintf(fout, "#ifndef _%s_ADDON_FUNCTION_H_\n", uName);
    fprintf(fout, "#define _%s_ADDON_FUNCTION_H_\n", uName);
    fprintf(fout, "#include <fcitx/addon.h>\n");

    free(uName);

    FcitxFunctionSignatureInfo* sig = fcitx_function_signature_info_new();

    char* prefix = format_underscore_name(functionInfo->fcitxAddon.name, false);

    for (uint32_t i = 0; i < functionInfo->fcitxAddon.function->len; i++) {
        char* function = *(char**) functionInfo->fcitxAddon.function->data[i];
        if (!function[0]) {
            continue;
        }

        FcitxConfiguration* subConfig = fcitx_configuration_get(config, function, false);
        if (!subConfig) {
            continue;
        }

        // reset to default value
        fcitx_function_signature_info_load(sig, subConfig);
        char* actualReturnType = strchr(sig->function.returnType, '*') ? "void*" : sig->function.returnType;
        char sigchar = get_sigchar_from_string(actualReturnType);
        if (!actualReturnType) {
            continue;
        }

        char* name = format_underscore_name(function, false);
        fprintf(fout, "// return %s\n", sig->function.returnType);
        for (uint32_t j = 0; j < sig->function.arg->len; j++) {
            char* typename = *fcitx_ptr_array_index(sig->function.arg, j, char*);
            fprintf(fout, "// %s arg%d\n", typename, j);
        }

        fprintf(fout, "static inline %s %s_invoke_%s(", actualReturnType, prefix, name);
        free(name);
        fprintf(fout, "FcitxAddonManager* manager");
        for (uint32_t j = 0; j < sig->function.arg->len; j++) {
            char* typename = *fcitx_ptr_array_index(sig->function.arg, j, char*);
            char* type = strchr(typename, '*') ? "void*" : typename;
            fprintf(fout, ", %s arg%d", type, j);
        }
        fprintf(fout, ")\n");
        fprintf(fout, "{\n");
        bool doreturn = strcmp(sig->function.returnType, "void") != 0;
        if (doreturn) {
            fprintf(fout, "    FcitxAddonFunctionArgument retVal;\n", actualReturnType);
            fprintf(fout, "    fcitx_addon_manager_invoke(manager, \"%s\", \"%s\", &retVal", functionInfo->fcitxAddon.name, function);
        } else {
            fprintf(fout, "    fcitx_addon_manager_invoke(manager, \"%s\", \"%s\", NULL", functionInfo->fcitxAddon.name, function);
        }
        for (uint32_t j = 0; j < sig->function.arg->len; j++) {
            fprintf(fout, ", arg%d", j);
        }
        fprintf(fout, ");\n");
        if (doreturn) {
            char* fieldname = NULL;
#define CHAR_TO_FIELD_NAME(CHAR, FIELD) \
    case CHAR: \
        fieldname = FIELD; \
        break;
            switch(sigchar) {
                CHAR_TO_FIELD_NAME('a', "a")
                CHAR_TO_FIELD_NAME('y', "s8")
                CHAR_TO_FIELD_NAME('n', "s16")
                CHAR_TO_FIELD_NAME('i', "s32")
                CHAR_TO_FIELD_NAME('x', "s64")
                CHAR_TO_FIELD_NAME('z', "u8")
                CHAR_TO_FIELD_NAME('q', "u16")
                CHAR_TO_FIELD_NAME('u', "u32")
                CHAR_TO_FIELD_NAME('t', "u64")
                CHAR_TO_FIELD_NAME('f', "f")
                CHAR_TO_FIELD_NAME('d', "d")
            }
            fprintf(fout, "    return retVal.%s;\n", fieldname);
        }
        fprintf(fout, "}\n");
    }

    free(prefix);

    fcitx_function_signature_info_free(sig);
    fprintf(fout, "#endif\n");
}

void print_c_header_internal(FcitxFunctionInfo* functionInfo, FcitxConfiguration* config)
{
    // print header guard

    char* uName = format_underscore_name(functionInfo->fcitxAddon.name, true);

    fprintf(fout, "#ifndef _%s_INTERNAL_H_\n", uName);
    fprintf(fout, "#define _%s_INTERNAL_H_\n", uName);
    fprintf(fout, "#include <fcitx-utils/utils.h>\n");

    free(uName);

    FcitxFunctionSignatureInfo* sig = fcitx_function_signature_info_new();

    char* prefix = format_underscore_name(functionInfo->fcitxAddon.name, false);

    for (uint32_t i = 0; i < functionInfo->fcitxAddon.include->len; i++) {
        char* include = *(char**) functionInfo->fcitxAddon.include->data[i];
        if (include[0] != '<' && include[0] != '\"') {
            fprintf(fout, "#include \"%s\"\n", include);
        } else {
            fprintf(fout, "#include %s\n", include);
        }
    }

    for (uint32_t i = 0; i < functionInfo->fcitxAddon.function->len; i++) {
        char* function = *(char**) functionInfo->fcitxAddon.function->data[i];
        if (!function[0]) {
            continue;
        }

        FcitxConfiguration* subConfig = fcitx_configuration_get(config, function, false);
        if (!subConfig) {
            continue;
        }

        // reset to default value
        fcitx_function_signature_info_load(sig, subConfig);

        char* name = format_underscore_name(function, false);
        fprintf(fout, "%s %s_%s(", sig->function.returnType, prefix, name);
        free(name);
        fprintf(fout, "%s* self", functionInfo->fcitxAddon.selfType);
        for (uint32_t j = 0; j < sig->function.arg->len; j++) {
            char* typename = *fcitx_ptr_array_index(sig->function.arg, j, char*);
            fprintf(fout, ", %s arg%d", typename, j);
        }
        fprintf(fout, ");\n");
    }
    fprintf(fout, "FcitxDict* %s_register_functions(void);\n", prefix);

    free(prefix);

    fcitx_function_signature_info_free(sig);

    fprintf(fout, "#endif\n");
}

char get_sigchar_from_string(const char* type)
{
    // we steal dbus signature here
    if (strchr(type, '*')) {
        return 'a';
    }

#define COMPARE_TYPE_NAME(array, CHAR) \
    do { \
        bool flag = false; \
        for (size_t i = 0; i < FCITX_ARRAY_SIZE(array); i++) { \
            if (strcmp(array[i], type) == 0) { \
                flag = true; \
                break; \
            } \
        } \
        if (flag) { \
            return CHAR; \
        } \
    } while(0)

    // 1-byte signed
    char* one_byte_signed_name[] = {
        "int8_t",
        "char",
    };
    // 2-byte signed
    char* two_byte_signed_name[] = {
        "int16_t",
#if SHRT_MAX == 32767
        "short",
#endif
#if INT_MAX == 32767
        "int",
#endif
    };
    // 4-byte signed
    char* four_byte_signed_name[] = {
        "int32_t",
#if SHRT_MAX == 2147483647
        "short",
#endif
#if INT_MAX == 2147483647
        "int",
#endif
#if LONG_MAX == 2147483647
        "long",
#endif
    };
    // 8-byte signed
    char* eight_byte_signed_name[] = {
        "int64_t",
#if INT_MAX == 9223372036854775807
        "int",
#endif
#if LONG_MAX == 9223372036854775807
        "long",
#endif
    };
    // 1-byte unsigned
    char* one_byte_unsigned_name[] = {
        "uint8_t",
        "unsigned char",
    };
    // 2-byte unsigned
    char* two_byte_unsigned_name[] = {
        "uint16_t",
#if SHRT_MAX == 32767
        "unsigned short",
#endif
#if INT_MAX == 32767
        "unsigned int",
#endif
    };
    // 4-byte unsigned
    char* four_byte_unsigned_name[] = {
        "uint32_t",
#if SHRT_MAX == 2147483647
        "unsigned short",
#endif
#if INT_MAX == 2147483647
        "unsigned int",
#endif
#if LONG_MAX == 2147483647
        "unsigned long",
#endif
    };
    // 8-byte unsigned
    char* eight_byte_unsigned_name[] = {
        "uint64_t",
#if INT_MAX == 9223372036854775807
        "unsigned int",
#endif
#if LONG_MAX == 9223372036854775807
        "unsigned long",
#endif
    };
    // float
    char* float_name[] = {
        "float",
    };
    // double
    char* double_name[] = {
        "double",
    };
    // void
    char* void_name[] = {
        "void",
    };

    if (strcmp("bool", type) == 0) {
        if (sizeof(bool) == 1) {
            return 'y';
        }
        if (sizeof(bool) == 2) {
            return 'n';
        }
        if (sizeof(bool) == 4) {
            return 'i';
        }
        if (sizeof(bool) == 8) {
            return 'x';
        }

        return '\0';
    }

    COMPARE_TYPE_NAME(one_byte_signed_name, 'y');
    COMPARE_TYPE_NAME(two_byte_signed_name, 'n');
    COMPARE_TYPE_NAME(four_byte_signed_name, 'i');
    COMPARE_TYPE_NAME(eight_byte_signed_name, 'x');
    COMPARE_TYPE_NAME(one_byte_unsigned_name, 'z');
    COMPARE_TYPE_NAME(two_byte_unsigned_name, 'q');
    COMPARE_TYPE_NAME(four_byte_unsigned_name, 'u');
    COMPARE_TYPE_NAME(eight_byte_unsigned_name, 't');
    COMPARE_TYPE_NAME(float_name, 'f');
    COMPARE_TYPE_NAME(double_name, 'd');
    COMPARE_TYPE_NAME(void_name, 'v');

    return '\0';
}

char* generate_signature_string(FcitxFunctionSignatureInfo* sig)
{
    char* sigstr = fcitx_utils_newv(char, sig->function.arg->len + 2);
    sigstr[0] = get_sigchar_from_string(sig->function.returnType);

    for (uint32_t i = 0; i < sig->function.arg->len; i++) {
        char* typename = *fcitx_ptr_array_index(sig->function.arg, i, char*);
        sigstr[i + 1] = get_sigchar_from_string(typename);
    }

    if (strlen(sigstr) !=  sig->function.arg->len + 1) {
        free(sigstr);
        return NULL;
    }

    return sigstr;
}

void print_c_source(FcitxFunctionInfo* functionInfo, FcitxConfiguration* config)
{
    fprintf(fout, "#include <fcitx/addon.h>\n");
    fprintf(fout, "#include \"%s-internal.h\"\n", functionInfo->fcitxAddon.name);

    FcitxFunctionSignatureInfo* sig = fcitx_function_signature_info_new();

    char* prefix = format_underscore_name(functionInfo->fcitxAddon.name, false);
    fprintf(fout, "FcitxDict* %s_register_functions(void)\n", prefix);
    fprintf(fout,
            "{\n"
            "    FcitxDict* functions = fcitx_dict_new(free);\n"
            "    FcitxAddonFunctionEntry* entry;\n");

    for (uint32_t i = 0; i < functionInfo->fcitxAddon.function->len; i++) {
        char* function = *(char**) functionInfo->fcitxAddon.function->data[i];
        if (!function[0]) {
            continue;
        }

        FcitxConfiguration* subConfig = fcitx_configuration_get(config, function, false);
        if (!subConfig) {
            continue;
        }

        // reset to default value
        fcitx_function_signature_info_load(sig, subConfig);

        char* sigstr = generate_signature_string(sig);
        if (sigstr) {
            char* name = format_underscore_name(function, false);
            fprintf(fout, "    entry = fcitx_utils_new_with_str(FcitxAddonFunctionEntry, \"%s\");\n", sigstr);
            fprintf(fout, "    entry->function = (FcitxCallback) %s_%s;\n", prefix, name);
            fprintf(fout, "    fcitx_dict_insert_by_str(functions, \"%s\", entry, true);\n", function);
            free(sigstr);
            free(name);
        }
    }
    fprintf(fout,
            "    return functions;\n"
            "}\n");

    free(prefix);

    fcitx_function_signature_info_free(sig);
}

void usage()
{
    fprintf(stderr, "\n");
}

int main(int argc, char* argv[])
{
    FcitxAddonFunctionCompileOperation op = Operation_C_Header;
    int c;
    while ((c = getopt(argc, argv, "ich")) != EOF) {
        switch (c) {
            case 'i':
                op = Operation_C_Header_Internal;
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

    if (argc <= optind) {
        usage();
        exit(0);
    }

    int result = 0;
    FILE* fin = NULL;

    do {
        fin = fopen(argv[optind], "r");
        if (!fin) {
            perror(NULL);
            result = 1;
            break;
        }

        if (argc > optind + 1) {
            fout = fopen(argv[optind + 1], "w");
        } else {
            fout = stdout;
        }

        if (!fout) {
            perror(NULL);
            result = 1;
            break;
        }

        FcitxConfiguration* config = fcitx_ini_parse(fin, NULL);
        FcitxFunctionInfo* functionInfo = fcitx_function_info_new();
        fcitx_function_info_load(functionInfo, config);

        // non-empty
        if (functionInfo->fcitxAddon.name[0]
         && functionInfo->fcitxAddon.selfType[0]) {
            if (op == Operation_C_Header) {
                print_c_header(functionInfo, config);
            } else if (op == Operation_C_Header_Internal) {
                print_c_header_internal(functionInfo, config);
            } else if (op == Operation_C_Source) {
                print_c_source(functionInfo, config);
            }
        }

        fcitx_function_info_free(functionInfo);

        fcitx_configuration_unref(config);
    } while(0);

    if (fin) {
        fclose(fin);
    }

    if (fout) {
        fclose(fout);
    }

    return result;
}
