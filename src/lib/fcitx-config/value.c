#include "value.h"
#include <stddef.h>

void* fcitx_value_alloc(FcitxConfigurationValueType type)
{
    size_t size = 0;
    switch(type) {
        case FCVT_Custom:
        case FCVT_Integer:
        case FCVT_String:
        case FCVT_File:
        case FCVT_Font:
        case FCVT_I18NString:
            size = sizeof(char*);
            break;
        case FCVT_Char:
            size = sizeof(char);
            break;
        case FCVT_Color:
            size = sizeof(FcitxColor);
            break;
        case FCVT_Boolean:
            size = sizeof(boolean);
            break;
        case FCVT_Enum:
            size = sizeof(int);
            break;
        case FCVT_Key:
            
    }

}
