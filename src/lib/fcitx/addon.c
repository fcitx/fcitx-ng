#include <dlfcn.h>
#include "addon.h"
#include "addon-internal.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include "config.h"
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

void* _fcitx_library_get_symbol(FcitxLibrary* library, const char* addonName, const char* symbolName)
{
    char *p;
    char *escapedAddonName;
    fcitx_utils_alloc_cat_str(escapedAddonName, addonName, "_", symbolName);
    for (p = escapedAddonName;*p;p++) {
        if (*p == '-') {
            *p = '_';
        }
    }
    void *result = fcitx_library_resolve(library, escapedAddonName);
    free(escapedAddonName);
    return result;
}

bool fcitx_shared_library_resolve(FcitxAddon* addon, FcitxAddonManager* manager, void* data)
{
    FCITX_UNUSED(data);
    FcitxStandardPath* standardPath = fcitx_addon_manager_get_standard_path(manager);
    char* toFree = NULL;
    const char* libraryName = NULL;
    if (!fcitx_utils_string_ends_with(addon->config->addon.library, FCITX_LIBRARY_SUFFIX)) {
        asprintf(&toFree, "%s%s", addon->config->addon.library, FCITX_LIBRARY_SUFFIX);
        libraryName = toFree;
    }
    free(toFree);
    FcitxStandardPathFile* file = fcitx_standard_path_locate(standardPath, FSPT_Addon, libraryName, 0);
    if (!file) {
        return false;
    }

    bool failed;
    FcitxLibrary* library = fcitx_library_new(file->path);
    do {
        if (!fcitx_library_load(library, FLLH_ResolveAllSymbolsHint | FLLH_ExportExternalSymbolsHint)) {
            failed = true;
            break;
        }

        int* version = _fcitx_library_get_symbol(library, addon->config->addon.name, "ABI_VERSION");
        if (*version != FCITX_ABI_VERSION) {
            failed = true;
            break;
        }
    } while(0);
    fcitx_standard_path_file_close(file);
    if (failed) {
        fcitx_library_free(library);
    }

    return false;
}

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
    mananger->addons = fcitx_dict_new((FcitxDestroyNotify) fcitx_addon_free);
    return fcitx_addon_manager_ref(mananger);
}

void fcitx_addon_manager_free(FcitxAddonManager* manager)
{
    fcitx_dict_free(manager->resolvers);
    fcitx_standard_path_unref(manager->standardPath);
    fcitx_dict_free(manager->addons);
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxAddonManager, fcitx_addon_manager);

FCITX_EXPORT_API
void fcitx_addon_manager_register_resolver(FcitxAddonManager* manager, const char* name, FcitxAddonResolver* resolver)
{
    fcitx_dict_insert_by_str(manager->resolvers, name, resolver, false);
}

FCITX_EXPORT_API
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

    if (addon->config->addon.name[0] == 0 || addon->config->addon.type[0] == 0) {
        fcitx_addon_free(addon);
        addon = NULL;
    }

    if (addon) {
        FcitxAddonManager* manager = userData;
        if (fcitx_dict_insert_by_str(manager->addons, addon->config->addon.name, addon, false)) {
        }
    }

    return false;
}

bool _fcitx_addon_resolve(FcitxAddon* addon, FcitxAddonResolver* resolver, FcitxAddonManager* manager)
{
    return resolver->resolve(addon, manager, resolver->data);
}

bool _fcitx_addon_manager_load_resolvers(const char* key, size_t keyLen, void** data, void* arg)
{
    FCITX_UNUSED(key);
    FCITX_UNUSED(keyLen);
    FcitxAddonManager* manager = arg;
    FcitxAddon* addon = *(FcitxAddon**) data;
    if (addon->config->addon.category != FAC_AddonResolver) {
        return false;
    }

    FcitxAddonResolver* resolver;
    if (!fcitx_dict_lookup_by_str(manager->resolvers, addon->config->addon.type, &resolver)) {
        return false;
    }

    return false;
}

FCITX_EXPORT_API
void fcitx_addon_manager_load(FcitxAddonManager* manager)
{
    // list all metadata
    FcitxStandardPathFilter filter;
    filter.flag = FSPFT_Suffix | FSPFT_Sort;
    filter.suffix = ".conf";
    FcitxDict* fileDict = fcitx_standard_path_match(manager->standardPath, FSPT_Data, "fcitx/addon", &filter);
    fcitx_dict_foreach(fileDict, _fcitx_addon_load, manager);
    fcitx_dict_free(fileDict);

    // load addon resolvers
    fcitx_dict_foreach(manager->addons, _fcitx_addon_manager_load_resolvers, NULL);
}

FcitxStandardPath* fcitx_addon_manager_get_standard_path(FcitxAddonManager* manager)
{
    return manager->standardPath;
}
