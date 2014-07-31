#ifndef _FCITX_CONFIG_DESCRIPTION_H_
#define _FCITX_CONFIG_DESCRIPTION_H_

#include "configuration.h"

typedef struct _FcitxDescription FcitxDescription;

struct _FcitxDescription {
    bool error;
    char* errorMessage;
    char* localeDomain;
    FcitxConfiguration* rootConfig;
    FcitxStringHashSet* structs;
    FcitxStringHashSet* topLevelStructs;
};

FcitxDescription* fcitx_description_parse(FcitxConfiguration* config);

void fcitx_description_free(FcitxDescription* desc);

#endif // _FCITX_CONFIG_DESCRIPTION_H_
