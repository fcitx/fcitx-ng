#ifndef _FCITX_CONFIG_HELPER_H_
#define _FCITX_CONFIG_HELPER_H_

#include "configuration.h"

void fcitx_configuration_get_string(FcitxConfiguration* config, const char* path, const char* defaultValue, char** str);

void fcitx_configuration_set_string(FcitxConfiguration* config, const char* path, const char* str);

void fcitx_configuration_get_integer(FcitxConfiguration* config, const char* path, const char* defaultValue, int* integer);

void fcitx_configuration_set_integer(FcitxConfiguration* config, const char* path, int integer);

void fcitx_configuration_get_boolean(FcitxConfiguration* config, const char* path, const char* defaultValue, bool *b);

void fcitx_configuration_set_boolean(FcitxConfiguration* config, const char* path, bool b);

void fcitx_configuration_get_char(FcitxConfiguration* config, const char* path, const char* defaultValue, char* chr);

void fcitx_configuration_set_char(FcitxConfiguration* config, const char* path, char chr);

void fcitx_configuration_get_color(FcitxConfiguration* config, const char* path, const char* defaultValue, FcitxColor* color);

void fcitx_configuration_set_color(FcitxConfiguration* config, const char* path, const FcitxColor* color);

void fcitx_configuration_get_key(FcitxConfiguration* config, const char* path, const char* defaultValue, FcitxKeyList** keyList);

void fcitx_configuration_set_key(FcitxConfiguration* config, const char* path, FcitxKeyList* keyList);

void fcitx_configuration_get_i18n_string(FcitxConfiguration* config, const char* path, const char* defaultValue, FcitxI18NString** str);

void fcitx_configuration_set_i18n_string(FcitxConfiguration* config, const char* path, FcitxI18NString* str);

void fcitx_configuration_get_enum(FcitxConfiguration* config, const char* path, const char** enumStrings, size_t enumCount, uint32_t defaultValue, uint32_t* enumValue);

void fcitx_configuration_set_enum(FcitxConfiguration* config, const char* path, const char** enumStrings, size_t enumCount, uint32_t enumValue);

#endif // _FCITX_CONFIG_HELPER_H_
