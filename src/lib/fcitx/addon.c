#include <dlfcn.h>
#include "addon.h"
#include "addon-internal.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include "config.h"
#include "addon-config.h"

typedef struct {
    char* name;
    FcitxStringHashSet* dependencies;
    FcitxListHead list;
} FcitxAddonTopoSortItem;

static const char* addonType[] = {
    "inputmethod",
    "frontend",
    "module",
    "ui",
    "addonresolver"
};

static bool fcitx_shared_library_resolve(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);
static void fcitx_shared_library_unload(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);

FcitxAddonResolver sharedLibraryResolver = {
    fcitx_shared_library_resolve,
    fcitx_shared_library_unload,
    NULL,
    NULL
};

FcitxAddonResolver staticLibraryResolver = {
    NULL,
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

bool fcitx_shared_library_resolve(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data)
{
    FCITX_UNUSED(data);
    FcitxStandardPath* standardPath = fcitx_addon_manager_get_standard_path(manager);
    char* toFree = NULL;
    const char* libraryName = NULL;
    if (!fcitx_utils_string_ends_with(addonConfig->addon.library, FCITX_LIBRARY_SUFFIX)) {
        asprintf(&toFree, "%s%s", addonConfig->addon.library, FCITX_LIBRARY_SUFFIX);
        libraryName = toFree;
    } else {
        libraryName = addonConfig->addon.library;
    }
    FcitxStandardPathFile* file = fcitx_standard_path_locate(standardPath, FSPT_Addon, libraryName, 0);
    free(toFree);
    if (!file) {
        return false;
    }

    bool result = false;
    FcitxLibrary* library = fcitx_library_new(file->path);
    do {
        if (!fcitx_library_load(library, FLLH_ResolveAllSymbolsHint | FLLH_PreventUnloadHint | FLLH_ExportExternalSymbolsHint)) {
            break;
        }

        int* version = _fcitx_library_get_symbol(library, addonConfig->addon.name, "ABI_VERSION");
        if (*version != FCITX_ABI_VERSION) {
            break;
        }

        if (addonConfig->addon.category >= FCITX_ARRAY_SIZE(addonType)) {
            break;
        }

        addonInst->api = _fcitx_library_get_symbol(library, addonConfig->addon.name, addonType[addonConfig->addon.category]);
        if (!addonInst->api) {
            break;
        }

        FcitxAddonAPICommon* apiCommon = addonInst->api;
        if (!apiCommon->init && !apiCommon->destroy) {
            break;
        }
        addonInst->data = apiCommon->init(manager);
        if (!addonInst->data) {
            break;
        }

        addonInst->resolverData = library;
        result = true;
    } while(0);
    fcitx_standard_path_file_close(file);
    if (!result) {
        fcitx_library_free(library);
    }

    return result;
}

void fcitx_shared_library_unload(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data)
{
    FCITX_UNUSED(addonConfig);
    FCITX_UNUSED(manager);
    FCITX_UNUSED(data);
    FcitxAddonAPICommon* apiCommon = addonInst->api;
    apiCommon->destroy(addonInst->data);
    fcitx_library_unload(addonInst->resolverData);
    fcitx_library_free(addonInst->resolverData);
}


void fcitx_addon_free(FcitxAddon* addon)
{
    fcitx_addon_config_free(addon->config);
    fcitx_string_hashset_free(addon->dependencies);
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
    FcitxAddonManager* manager = fcitx_utils_new(FcitxAddonManager);
    manager->resolvers = fcitx_dict_new(fcitx_addon_resolver_free);
    manager->standardPath = fcitx_standard_path_ref(standardPath);
    manager->addons = fcitx_dict_new((FcitxDestroyNotify) fcitx_addon_free);
    manager->loadedAddons = fcitx_ptr_array_new(NULL);
    manager->properties = fcitx_dict_new(NULL);
    return fcitx_addon_manager_ref(manager);
}

void fcitx_addon_manager_free(FcitxAddonManager* manager)
{
    if (manager->loaded) {
        fcitx_addon_manager_unload(manager);
    }
    fcitx_dict_free(manager->properties);
    fcitx_ptr_array_free(manager->loadedAddons);
    fcitx_dict_free(manager->resolvers);
    fcitx_standard_path_unref(manager->standardPath);
    fcitx_dict_free(manager->addons);
    fcitx_string_hashset_free(manager->enabledAddons);
    fcitx_string_hashset_free(manager->disabledAddons);
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxAddonManager, fcitx_addon_manager);

FCITX_EXPORT_API
void fcitx_addon_manager_register_resolver(FcitxAddonManager* manager, const char* name, FcitxAddonResolver* resolver)
{
    fcitx_dict_insert_by_str(manager->resolvers, name, resolver, false);
}

FCITX_EXPORT_API
void fcitx_addon_manager_set_property(FcitxAddonManager* manager, const char* name, void* data)
{
    fcitx_dict_insert_by_str(manager->properties, name, data, true);
}

FCITX_EXPORT_API
void* fcitx_addon_manager_get_property(FcitxAddonManager* manager, const char* name)
{
    void* result = NULL;
    fcitx_dict_lookup_by_str(manager->properties, name, &result);
    return result;
}


FCITX_EXPORT_API
void fcitx_addon_manager_set_override(FcitxAddonManager* manager, const char* enabled, const char* disabled)
{
    fcitx_string_hashset_free(manager->enabledAddons);
    fcitx_string_hashset_free(manager->disabledAddons);

    manager->enabledAddons = enabled ? fcitx_string_hashset_parse(enabled, ',') : NULL;
    manager->disabledAddons = disabled ? fcitx_string_hashset_parse(disabled, ',') : NULL;
    manager->disabledAllAddons = manager->disabledAddons ? fcitx_string_hashset_contains(manager->disabledAddons, "all") : false;

}

FCITX_EXPORT_API
void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger)
{
    fcitx_dict_insert_by_str(mananger->resolvers, "StaticLibrary", &staticLibraryResolver, false);
    fcitx_dict_insert_by_str(mananger->resolvers, "SharedLibrary", &sharedLibraryResolver, false);
}

bool _fcitx_addon_load_metadata(const char* key, size_t keyLen, void** data, void* userData)
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

    FcitxAddonConfig* addonConfig = fcitx_addon_config_new();
    fcitx_addon_config_load(addonConfig, config);
    fcitx_configuration_unref(config);

    do {
        if (addonConfig->addon.name[0] == 0 || addonConfig->addon.type[0] == 0 ||
            addonConfig->addon.category >= FAC_Last) {
            fcitx_addon_config_free(addonConfig);
            break;
        }

        FcitxAddonManager* manager = userData;
        FcitxAddon* addon = NULL;
        if (fcitx_dict_lookup_by_str(manager->addons, addonConfig->addon.name, &addon)) {
            // don't touch the running addon
            if (addon->loaded) {
                fcitx_addon_config_free(addonConfig);
                addon = NULL;
                break;
            } else {
                fcitx_addon_config_free(addon->config);
                addon->config = addonConfig;
            }
        } else {
            addon = fcitx_utils_new(FcitxAddon);
            addon->config = addonConfig;
            fcitx_dict_insert_by_str(manager->addons, addon->config->addon.name, addon, false);
        }

        if (addon) {
            addon->dependencies = fcitx_string_hashset_parse(addon->config->addon.dependency, ',');
        }

        if (addon &&
            (manager->disabledAllAddons ||
             (manager->disabledAllAddons && fcitx_string_hashset_contains(manager->disabledAddons, addon->config->addon.name)))) {
            addon->config->addon.enabled = false;
        }

        if (addon &&
            (manager->enabledAddons && fcitx_string_hashset_contains(manager->enabledAddons, addon->config->addon.name))) {
            addon->config->addon.enabled = true;
        }

    } while(0);

    return false;
}

void _fcitx_addon_manager_load_addon(FcitxAddonManager* manager, FcitxAddon* addon)
{
    if (addon->loaded) {
        return;
    }

    FcitxAddonResolver* resolver;

    do {
        if (!fcitx_dict_lookup_by_str(manager->resolvers, addon->config->addon.type, &resolver)) {
            break;
        }

        if (!resolver->resolve(addon->config, &addon->inst, manager, resolver->data)) {
            break;
        }
        fcitx_ptr_array_append(manager->loadedAddons, addon);
        return;
    } while(0);

    addon->config->addon.enabled = false;
}

void _fcitx_addon_manager_unload_addon(FcitxAddonManager* manager, FcitxAddon* addon)
{
    FcitxAddonResolver* resolver;
    fcitx_dict_lookup_by_str(manager->resolvers, addon->config->addon.type, &resolver);
    resolver->unload(addon->config, &addon->inst, manager, resolver->data);
}

FCITX_EXPORT_API
void fcitx_addon_manager_load(FcitxAddonManager* manager)
{
    // list all metadata
    FcitxStandardPathFilter filter;
    filter.flag = FSPFT_Suffix | FSPFT_Sort;
    filter.suffix = ".conf";
    FcitxDict* fileDict = fcitx_standard_path_match(manager->standardPath, FSPT_Data, "fcitx/addon", &filter);
    fcitx_dict_foreach(fileDict, _fcitx_addon_load_metadata, manager);
    fcitx_dict_free(fileDict);

    // topo sort doesn't work here, since
    bool update;
    do {
        update = false;
        for (FcitxDictData* data = fcitx_dict_first(manager->addons);
            data != NULL;
            data = fcitx_dict_data_next(data)) {
            FcitxAddon* addon = data->data;
            if (!addon->config->addon.enabled) {
                continue;
            }
            FcitxDictData* dependency = NULL;
            bool allDependenciesLoaded = true;
            for (dependency = fcitx_dict_first(addon->dependencies);
                dependency != NULL;
                dependency = fcitx_dict_data_next(dependency)) {
                FcitxAddon* dependAddon;
                if (!fcitx_dict_lookup_by_str(manager->addons, dependency->key, &dependAddon)) {
                    break;
                }
                if (!dependAddon->loaded) {
                    allDependenciesLoaded = false;
                }
            }

            if (dependency) {
                addon->config->addon.enabled = false;
                update = true;
            } else if (allDependenciesLoaded) {
                _fcitx_addon_manager_load_addon(manager, addon);
            }
        }
    } while(update);

    manager->loaded = true;
}

FCITX_EXPORT_API
FcitxStandardPath* fcitx_addon_manager_get_standard_path(FcitxAddonManager* manager)
{
    return manager->standardPath;
}

FCITX_EXPORT_API
void fcitx_addon_manager_unload(FcitxAddonManager* manager)
{
    if (!manager->loaded) {
        return;
    }

    for (size_t i = 0; i < manager->loadedAddons->len; i++) {
        FcitxAddon* addon = manager->loadedAddons->data[i];
        _fcitx_addon_manager_unload_addon(manager, addon);
        addon->loaded = false;
    }
    fcitx_ptr_array_clear(manager->loadedAddons);
}
