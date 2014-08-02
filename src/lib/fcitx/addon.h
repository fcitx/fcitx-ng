#ifndef __FCITX_ADDON_H__
#define __FCITX_ADDON_H__
#include <fcitx-utils/utils.h>

/**
 * Addon Category Definition
 **/
typedef enum _FcitxAddonCategory {
    /**
     * Input method
     **/
    FAC_InputMethod = 0,
    /**
     * Input frontend, like xim
     **/
    FAC_Frontend,
    /**
     * General module, can be implemented in a quite extensive way
     **/
    FAC_Module,
    /**
     * User Interface, only one of it can be enabled currently.
     **/
    FAC_UI,
    /**
     * addon that loads other addon
     */
    FAC_AddonResolver
} FcitxAddonCategory;

typedef struct _FcitxAddon FcitxAddon;

typedef bool (*FcitxAddonInitFunc)();
typedef void (*FcitxAddonDestroyFunc)();

typedef struct _FcitxAddonCommon
{
    FcitxAddonInitFunc init;
    FcitxAddonDestroyFunc destroy;
    void* padding[10];
} FcitxAddonCommon;

typedef struct _FcitxAddonInstance {
    FcitxAddon* addon;

    void* userData;
} FcitxAddonInstance;

typedef struct _FcitxAddonMananger FcitxAddonManager;

#define FCITX_ABI_VERSION 6

typedef bool (*FcitxAddonResolveFunctionFunc)(FcitxAddon* addon, FcitxAddonManager* manager, void* data);

typedef struct _FcitxAddonResolver {
    FcitxAddonResolveFunctionFunc resolve;
    FcitxDestroyNotify destroyNotify;
    void *data;
} FcitxAddonResolver;

FcitxAddonManager* fcitx_addon_manager_new(FcitxStandardPath* standardPath);
FcitxAddonManager* fcitx_addon_manager_ref(FcitxAddonManager* manager);
void fcitx_addon_manager_unref(FcitxAddonManager* manager);
void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger);
void fcitx_addon_manager_load(FcitxAddonManager* manager);
FcitxStandardPath* fcitx_addon_manager_get_standard_path(FcitxAddonManager* manager);

#endif // __FCITX_ADDON_H__
