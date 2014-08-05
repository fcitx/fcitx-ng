#ifndef __FCITX_ADDON_H__
#define __FCITX_ADDON_H__
#include <fcitx-utils/utils.h>
#include "addon-config.h"


typedef struct _FcitxAddonMananger FcitxAddonManager;

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
    FAC_AddonResolver,
    FAC_Last
} FcitxAddonCategory;

typedef struct _FcitxAddon FcitxAddon;

typedef void* (*FcitxAddonInitFunc)(FcitxAddonManager* manager);
typedef void (*FcitxAddonDestroyFunc)(void* addon);

typedef struct _FcitxAddonAPICommon
{
    FcitxAddonInitFunc init;
    FcitxAddonDestroyFunc destroy;
    void* padding1;
    void* padding2;
    void* padding3;
    void* padding4;
    void* padding5;
    void* padding6;
} FcitxAddonAPICommon;

typedef struct _FcitxAddonInstance
{
    // Resolver need to fill this
    void* api;
    void* data;
    void* resolverData; // add a field for convinience
} FcitxAddonInstance;

#define FCITX_ABI_VERSION 6

#define FCITX_DEFINE_ADDON(name, category, type) \
FCITX_EXPORT_API int name##_ABI_VERSION = FCITX_ABI_VERSION; \
FCITX_EXPORT_API type name##_##category

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
void fcitx_addon_manager_set_property(FcitxAddonManager* manager, const char* name, void* data);
void* fcitx_addon_manager_get_property(FcitxAddonManager* manager, const char* name);
void fcitx_addon_manager_set_override(FcitxAddonManager* manager, const char* enabled, const char* disabled);
void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger);
void fcitx_addon_manager_load(FcitxAddonManager* manager);
void fcitx_addon_manager_unload(FcitxAddonManager* manager);
FcitxStandardPath* fcitx_addon_manager_get_standard_path(FcitxAddonManager* manager);

#endif // __FCITX_ADDON_H__
