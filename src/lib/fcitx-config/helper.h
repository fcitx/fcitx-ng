#ifndef _FCITX_CONFIG_HELPER_H_
#define _FCITX_CONFIG_HELPER_H_

#include "configuration.h"

typedef struct _FcitxConfigurationOptionInfo FcitxConfigurationOptionInfo;
typedef void (*FcitxConfigurationGetFunc)(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, void* data);
typedef void (*FcitxConfigurationSetFunc)(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, void* data);

struct _FcitxConfigurationOptionInfo
{
    union {
        struct {
            const char* defaultValue;
        } regular;

        struct {
            const char** enumStrings;
            size_t enumCount;
            uint32_t defaultValue;
        } enumeration;
    };

    union {
        struct {
            int min;
            int max;
        } integer;

        struct {
            size_t maxLength;
        } string;

        struct {
            bool disallowNoModifer;
            bool allowModifierOnly;
        } hotkey;

        void* padding[10];
    } constrain;

    struct {
        size_t size;
        FcitxConfigurationGetFunc loadFunc;
        FcitxConfigurationSetFunc storeFunc;
        FcitxDestroyNotify freeFunc;
    } list;
};


void fcitx_configuration_get_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, char** str);

void fcitx_configuration_set_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, char **str);

void fcitx_configuration_get_integer(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, int* integer);

void fcitx_configuration_set_integer(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const int* integer);

void fcitx_configuration_get_boolean(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, bool *b);

void fcitx_configuration_set_boolean(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const bool* b);

void fcitx_configuration_get_char(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, char* chr);

void fcitx_configuration_set_char(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const char* chr);

void fcitx_configuration_get_color(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxColor* color);

void fcitx_configuration_set_color(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, const FcitxColor* color);

void fcitx_configuration_get_key(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxKeyList** keyList);

void fcitx_configuration_set_key(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxKeyList** keyList);

void fcitx_configuration_get_i18n_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxI18NString** str);

void fcitx_configuration_set_i18n_string(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxI18NString** str);

void fcitx_configuration_get_enum(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, uint32_t* enumValue);

void fcitx_configuration_set_enum(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, uint32_t* enumValue);

void fcitx_configuration_get_list(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxPtrArray** list);

void fcitx_configuration_set_list(FcitxConfiguration* config, const char* path, FcitxConfigurationOptionInfo* info, FcitxPtrArray** list);

#endif // _FCITX_CONFIG_HELPER_H_

