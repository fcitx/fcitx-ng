#ifndef FCITX_CONFIG_CONFIGURATION_H
#define FCITX_CONFIG_CONFIGURATION_H

#include "configurationbackend.h"
#include <fcitx-utils/types.h>
#include <fcitx-utils/dict.h>

typedef enum _FcitxConfigurationItemType
{
    FCIT_Group,
    FCIT_Item
} FcitxConfigurationItemType;

typedef struct _FcitxConfiguration
{
    char* root;
    FcitxDict* subitems;
    char* name;
    char* value;
} FcitxConfiguration;

#endif // FCITX_CONFIG_CONFIGURATION_H
