#include "addon.h"
#include "addon-internal.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include "addon-config.h"

FcitxAddonResolver sharedLibraryResolver = {
    NULL,
    NULL,
    NULL
};

FcitxAddonResolver staticLibraryResolver = {
    NULL,
    NULL,
    NULL
};

FcitxAddon* fcitx_addon_new()
{
    FcitxAddon* addon = fcitx_utils_new(FcitxAddon);
    addon->config = fcitx_addon_config_new();
    return addon;
}

void fcitx_addon_free(FcitxAddon* addon)
{
    fcitx_addon_config_free(addon->config);
    free(addon);
}

void fcitx_addon_resolver_free(void* data)
{
    FcitxAddonResolver* resolver = data;
    if (resolver->destroyNotify) {
        resolver->destroyNotify(resolver);
    }
}

FCITX_EXPORT_API
FcitxAddonManager* fcitx_addon_manager_new(FcitxStandardPath* standardPath)
{
    FcitxAddonManager* mananger = fcitx_utils_new(FcitxAddonManager);
    mananger->resolvers = fcitx_dict_new(fcitx_addon_resolver_free);
    mananger->standardPath = fcitx_standard_path_ref(standardPath);
    mananger->addons = fcitx_ptr_array_new((FcitxDestroyNotify) fcitx_addon_free);
    return fcitx_addon_manager_ref(mananger);
}

void fcitx_addon_manager_free(FcitxAddonManager* manager)
{
    fcitx_dict_free(manager->resolvers);
    fcitx_standard_path_unref(manager->standardPath);
    fcitx_ptr_array_free(manager->addons);
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxAddonManager, fcitx_addon_manager);

FCITX_EXPORT_API
void fcitx_addon_manager_register_resolver(FcitxAddonManager* manager, const char* name, FcitxAddonResolver* resolver)
{
    fcitx_dict_insert_by_str(manager->resolvers, name, resolver, false);
}

void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger)
{
    fcitx_dict_insert_by_str(mananger->resolvers, "staticlibrary", &staticLibraryResolver, false);
    fcitx_dict_insert_by_str(mananger->resolvers, "sharedlibrary", &sharedLibraryResolver, false);
}

bool _fcitx_addon_load(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(key);
    FCITX_UNUSED(keyLen);
    FcitxStandardPathFile* files = *data;
    size_t i = 0;
    while (files[i].fp) {
        i++;
    }
    FcitxConfiguration* config = NULL;
    while (i != 0) {
        i --;
        config = fcitx_ini_parse(files[i].fp, config);
    }

    FcitxAddon* addon = fcitx_addon_new();
    fcitx_addon_config_load(addon->config, config);
    fcitx_configuration_unref(config);

    FcitxAddonManager* manager = userData;
    fcitx_ptr_array_append(manager->addons, addon);

    return false;
}

FCITX_EXPORT_API
void fcitx_addon_manager_load(FcitxAddonManager* manager)
{
    // load the metadata
    FcitxStandardPathFilter filter;
    filter.flag = FSPFT_Suffix | FSPFT_Sort;
    filter.suffix = ".conf";
    FcitxDict* fileDict = fcitx_standard_path_match(manager->standardPath, FSPT_Data, "fcitx/addon", &filter);
    fcitx_dict_foreach(fileDict, _fcitx_addon_load, manager);
    fcitx_dict_free(fileDict);

    //
}
