#ifndef __FCITX_ADDON_INTERNAL_H__
#define __FCITX_ADDON_INTERNAL_H__

#include <stdint.h>
#include "addon.h"
#include "addon-config.h"

/**
 * Addon Instance in Fcitx
 **/
struct _FcitxAddon {
    FcitxAddonConfig *config;
    bool loaded;
    FcitxStringHashSet* dependencies;
    FcitxAddonInstance inst;
};

struct _FcitxAddonMananger {
    FcitxDict* resolvers;
    FcitxStandardPath* standardPath;
    int32_t refcount;
    FcitxDict* addons;

    // override data;
    FcitxStringHashSet* enabledAddons;
    FcitxStringHashSet* disabledAddons;
    bool disabledAllAddons;

    // load order;
    FcitxPtrArray* loadedAddons;
    bool loaded;
    FcitxDict* properties;
};

#endif // __FCITX_ADDON_INTERNAL_H__
