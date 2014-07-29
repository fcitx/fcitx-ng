#include "addon.h"
#include "addon-internal.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
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

void fcitx_addon_resolver_free(void* data)
{
    FcitxAddonResolver* resolver = data;
    if (resolver->destroyNotify) {
        resolver->destroyNotify(resolver);
    }
}

FcitxAddonManager* fcitx_addon_manager_new(FcitxStandardPath* standardPath)
{
    FcitxAddonManager* mananger = fcitx_utils_new(FcitxAddonManager);
    mananger->resolvers = fcitx_dict_new(fcitx_addon_resolver_free);
    mananger->standardPath = fcitx_standard_path_ref(standardPath);
    return mananger;
}

FCITX_EXPORT_API
void fcitx_addon_manager_free(FcitxAddonManager* manager)
{
    fcitx_dict_free(manager->resolvers);
    fcitx_standard_path_unref(manager->standardPath);
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
    FcitxStandardPathFile* files = *data;
    size_t i = 0;
    while (files[i].fp) {
        i++;
    }
    i --;
    FcitxConfiguration* config = NULL;
    do {
        config = fcitx_ini_parse(files[i].fp, config);
        i --;
    } while(i != 0);


    return false;
}

void fcitx_addon_manager_load(FcitxAddonManager* manager)
{
    FcitxStandardPathFilter filter;
    filter.flag = FSPFT_Suffix | FSPFT_Sort;
    filter.suffix = ".conf";
    FcitxDict* fileDict = fcitx_standard_path_match(manager->standardPath, FSPT_Data, "fcitx/addon", &filter);
    fcitx_dict_foreach(fileDict, _fcitx_addon_load, manager);
}
