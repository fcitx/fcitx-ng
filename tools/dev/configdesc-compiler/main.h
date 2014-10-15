#ifndef _FCITX_TOOLS_CONFIGDESC_COMPILER_MAIN_H_
#define _FCITX_TOOLS_CONFIGDESC_COMPILER_MAIN_H_

#include "fcitx-utils/utils.h"
#include "fcitx-config/configuration.h"
#include "fcitx-config/description.h"

extern FILE* fout;

void compile_to_pot(FcitxConfiguration* config, FcitxDescription* desc);

void compile_to_c_source(FcitxConfiguration* config, FcitxDescription* desc, const char* name, const char* prefix, const char* includes);

void compile_to_c_header(FcitxConfiguration* config, FcitxDescription* desc, const char* name, const char* prefix, const char* includes);

void print_includes(const char* includes);

bool validate(FcitxConfiguration* config, FcitxStringHashSet* structs);

#endif // _FCITX_TOOLS_CONFIGDESC_COMPILER_MAIN_H_
