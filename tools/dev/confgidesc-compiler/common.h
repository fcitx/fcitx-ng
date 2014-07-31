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

void print_includes(const char* includes);

#endif // _FCITX_TOOLS_CONFIGDESC_COMPILER_COMMON_H_
