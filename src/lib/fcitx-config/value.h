#ifndef _FCITX_CONFIG_VALUE_H_
#define _FCITX_CONFIG_VALUE_H_
#include <stdint.h>
#include <fcitx-utils/types.h>

typedef enum _FcitxConfigurationValueType
{
    FCVT_Integer, // signed integer
    FCVT_Color, // 24bit color
    FCVT_String, // utf8 string
    FCVT_Char,
    FCVT_Boolean,
    FCVT_Enum,
    FCVT_File,  // simply string, but usefull for ui
    FCVT_Key,
    FCVT_Font,
    FCVT_I18NString,
    FCVT_Custom, // a string, which need to parsed by application
    FCVT_Last = FCVT_Custom
} FcitxConfigurationValueType;

void* fcitx_value_alloc(FcitxConfigurationValueType type);

#endif
