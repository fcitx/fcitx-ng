#ifndef __FCITX_ADDON_H__
#define __FCITX_ADDON_H__
#include <fcitx-utils/utils.h>
#include "addon-config.h"

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

typedef struct _FcitxAddonAPICommon
{
    FcitxAddonInitFunc init;
    FcitxAddonDestroyFunc destroy;
    void* padding[10];
} FcitxAddonAPICommon;

typedef struct _FcitxAddonAPI
{
    FcitxAddonAPICommon common;
} FcitxAddonAPI;

typedef struct _FcitxAddonInstance
{
    // Resolver need to fill this
    FcitxAddonAPI* api;
    void* data;
    void* resolverData; // add a field for convinience
} FcitxAddonInstance;

typedef struct _FcitxAddonMananger FcitxAddonManager;

#define FCITX_ABI_VERSION 6

typedef bool (*FcitxAddonResolveFunc)(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);
typedef void (*FcitxAddonUnloadFunc)(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);

typedef struct _FcitxAddonResolver {
    FcitxAddonResolveFunc resolve;
    FcitxAddonUnloadFunc unload;
    FcitxDestroyNotify destroyNotify;
    void *data;
} FcitxAddonResolver;

FcitxAddonManager* fcitx_addon_manager_new(FcitxStandardPath* standardPath);
FcitxAddonManager* fcitx_addon_manager_ref(FcitxAddonManager* manager);
void fcitx_addon_manager_unref(FcitxAddonManager* manager);
void fcitx_addon_manager_set_override(FcitxAddonManager* manager, const char* enabled, const char* disabled);
void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger);
void fcitx_addon_manager_load(FcitxAddonManager* manager);
void fcitx_addon_manager_unload(FcitxAddonManager* manager);
FcitxStandardPath* fcitx_addon_manager_get_standard_path(FcitxAddonManager* manager);

#endif // __FCITX_ADDON_H__
