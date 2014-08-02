#ifndef __FCITX_ADDON_INTERNAL_H__
#define __FCITX_ADDON_INTERNAL_H__

#include <stdint.h>
#include "addon.h"
#include "addon-config.h"


/**
 * How addon get input method list
 **/
typedef enum _IMRegisterMethod {
    IMRM_SELF,
    IMRM_EXEC,
    IMRM_CONFIGFILE
} IMRegisterMethod;

/**
 * Addon Instance in Fcitx
 **/
struct _FcitxAddon {
    FcitxAddonConfig *config;
};

struct _FcitxAddonMananger {
    FcitxDict* resolvers;
    FcitxStandardPath* standardPath;
    int32_t refcount;
    FcitxDict* addons;
};

#endif // __FCITX_ADDON_INTERNAL_H__
