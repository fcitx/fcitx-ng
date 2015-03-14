/*
 * Copyright (C) 2015~2015 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include <dlfcn.h>
#include <ffi.h>
#include <stdarg.h>
#include "addon.h"
#include "addon-internal.h"
#include "frontend.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include "config.h"
#include "addon-config.h"

static const char* addonType[] = {
    "inputmethod",
    "frontend",
    "module",
    "ui",
    "addonresolver"
};

static bool fcitx_shared_library_resolve(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);
static void fcitx_shared_library_unload(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);
static bool fcitx_static_library_resolve(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);
static void fcitx_static_library_unload(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data);
static void fcitx_static_library_resolver_free(void* data);

FCITX_EXPORT_API
bool fcitx_addon_init_general(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager)
{
    FcitxAddonAPICommon* apiCommon = addonInst->api;
    if (!apiCommon->init && !apiCommon->destroy) {
        return false;
    }

    if (addonConfig->addon.category == FAC_Frontend) {
        FcitxAddonAPIFrontend* apiFrontend = addonInst->api;
        apiFrontend->frontendId = fcitx_ptr_array_size(manager->frontends);
    }

    addonInst->data = apiCommon->init(manager, addonConfig);
    if (!addonInst->data) {
        return false;
    }

    if (addonConfig->addon.category == FAC_Frontend) {
        FcitxAddon* addon = fcitx_container_of(addonInst, FcitxAddon, inst);
        fcitx_ptr_array_append(manager->frontends, addon);
    }

    if (apiCommon->registerCallback) {
        addonInst->functions = apiCommon->registerCallback(addonInst->data);
    }

    return true;
}


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
        if (!version || *version != FCITX_ABI_VERSION) {
            break;
        }

        addonInst->api = _fcitx_library_get_symbol(library, addonConfig->addon.name, addonType[addonConfig->addon.category]);
        if (!addonInst->api) {
            break;
        }

        if (!fcitx_addon_init_general(addonConfig, addonInst, manager)) {
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
    fcitx_library_unload(addonInst->resolverData);
    fcitx_library_free(addonInst->resolverData);
}

bool fcitx_static_library_resolve(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data)
{
    FcitxDict* staticAddonDict = data;

    bool result = false;
    do {
        fcitx_dict_lookup_by_str(staticAddonDict, addonConfig->addon.name, &addonInst->api);
        if (!addonInst->api) {
            break;
        }

        if (!fcitx_addon_init_general(addonConfig, addonInst, manager)) {
            break;
        }

        addonInst->resolverData = NULL;
        result = true;
    } while(0);

    return result;
}

void fcitx_static_library_unload(const FcitxAddonConfig* addonConfig, FcitxAddonInstance* addonInst, FcitxAddonManager* manager, void* data)
{
    FCITX_UNUSED(addonConfig);
    FCITX_UNUSED(manager);
    FCITX_UNUSED(data);
    FCITX_UNUSED(addonInst);
}

void fcitx_static_library_resolver_free(void* data)
{
    FcitxDict* staticAddonDict = data;
    fcitx_dict_free(staticAddonDict);
}

FcitxAddon* fcitx_addon_new(FcitxAddonConfig* addonConfig)
{
    FcitxAddon* addon = fcitx_utils_new(FcitxAddon);
    addon->config = addonConfig;
    return addon;
}

void fcitx_addon_free(FcitxAddon* addon)
{
    fcitx_dict_free(addon->inst.functions);
    fcitx_addon_config_free(addon->config);
    fcitx_string_hashset_free(addon->dependencies);
    free(addon);
}

void fcitx_addon_resolver_free(void* data)
{
    FcitxAddonResolver* resolver = data;
    if (resolver->destroyNotify) {
        resolver->destroyNotify(resolver->data);
    }
    free(resolver);
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
    manager->frontends = fcitx_ptr_array_new(NULL);
    return fcitx_addon_manager_ref(manager);
}

void fcitx_addon_manager_free(FcitxAddonManager* manager)
{
    if (manager->loaded) {
        fcitx_addon_manager_unload(manager);
    }
    fcitx_ptr_array_free(manager->frontends);
    fcitx_dict_free(manager->properties);
    fcitx_ptr_array_free(manager->loadedAddons);
    fcitx_standard_path_unref(manager->standardPath);
    fcitx_dict_free(manager->addons);
    fcitx_dict_free(manager->resolvers);
    fcitx_string_hashset_free(manager->enabledAddons);
    fcitx_string_hashset_free(manager->disabledAddons);
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxAddonManager, fcitx_addon_manager);

FCITX_EXPORT_API
void fcitx_addon_manager_register_resolver(FcitxAddonManager* manager, const char* name, FcitxAddonResolver* resolver)
{
    FcitxAddonResolver* copiedResolver = fcitx_utils_new(FcitxAddonResolver);
    memcpy(copiedResolver, resolver, sizeof(FcitxAddonResolver));
    fcitx_dict_insert_by_str(manager->resolvers, name, copiedResolver, false);
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
void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger, FcitxStaticAddon* staticAddon)
{
    FcitxDict* staticAddonDict = fcitx_dict_new(NULL);
    while (staticAddon && staticAddon->name && staticAddon->entry) {
        fcitx_dict_insert_by_str(staticAddonDict, staticAddon->name, staticAddon->entry, false);
        staticAddon++;
    }

    FcitxAddonResolver staticLibraryResolver;
    memset(&staticLibraryResolver, 0, sizeof(staticLibraryResolver));
    staticLibraryResolver.destroyNotify = fcitx_static_library_resolver_free;
    staticLibraryResolver.resolve = fcitx_static_library_resolve;
    staticLibraryResolver.unload = fcitx_static_library_unload;
    staticLibraryResolver.data = staticAddonDict;

    FcitxAddonResolver sharedLibraryResolver;
    memset(&sharedLibraryResolver, 0, sizeof(sharedLibraryResolver));
    sharedLibraryResolver.resolve = fcitx_shared_library_resolve;
    sharedLibraryResolver.unload = fcitx_shared_library_unload;
    fcitx_addon_manager_register_resolver(mananger, "StaticLibrary", &staticLibraryResolver);
    fcitx_addon_manager_register_resolver(mananger, "SharedLibrary", &sharedLibraryResolver);
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
            addon = fcitx_addon_new(addonConfig);
            fcitx_dict_insert_by_str(manager->addons, addon->config->addon.name, addon, false);
        }

        if (addon) {
            // free old one
            fcitx_string_hashset_free(addon->dependencies);
            addon->dependencies = fcitx_string_hashset_parse(addon->config->addon.dependency, ',');
        }

        if (addon &&
            (manager->disabledAllAddons ||
             (manager->disabledAddons && fcitx_string_hashset_contains(manager->disabledAddons, addon->config->addon.name)))) {
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

        addon->loaded = true;
        fcitx_ptr_array_append(manager->loadedAddons, addon);

        return;
    } while(0);

    // load failed
    addon->config->addon.enabled = false;
}

void _fcitx_addon_manager_unload_addon(FcitxAddonManager* manager, FcitxAddon* addon)
{
    FcitxAddonAPICommon* apiCommon = addon->inst.api;
    apiCommon->destroy(addon->inst.data);
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

    // reverse against load order
    for (size_t i = manager->loadedAddons->len; i-- > 0; ) {
        FcitxAddon* addon = manager->loadedAddons->data[i];
        _fcitx_addon_manager_unload_addon(manager, addon);
        addon->loaded = false;
    }
    fcitx_ptr_array_clear(manager->loadedAddons);
}

void fcitx_arg_to_ffi_args(char c, FcitxAddonFunctionArgument* farg, ffi_type** ffitype, void** arg)
{
#define CHAR_TO_FFI_TYPE(CHAR, FIELD, FFI_TYPE, C_TYPE) \
    case CHAR: \
        *ffitype = &ffi_type_##FFI_TYPE; \
        if (arg) { \
            *arg = &farg->FIELD; \
        } \
        break;
    switch(c) {
        CHAR_TO_FFI_TYPE('a', a, pointer, void*)
        CHAR_TO_FFI_TYPE('y', s8, sint8, int8_t)
        CHAR_TO_FFI_TYPE('n', s16, sint16, int16_t)
        CHAR_TO_FFI_TYPE('i', s32, sint32, int32_t)
        CHAR_TO_FFI_TYPE('x', s64, sint64, int64_t)
        CHAR_TO_FFI_TYPE('z', u8, uint8, uint8_t)
        CHAR_TO_FFI_TYPE('q', u16, uint16, uint16_t)
        CHAR_TO_FFI_TYPE('u', u32, uint32, uint32_t)
        CHAR_TO_FFI_TYPE('t', u64, uint64, uint64_t)
        CHAR_TO_FFI_TYPE('f', f, float, float)
        CHAR_TO_FFI_TYPE('d', d, double, double)
        case 'v':
            *ffitype = &ffi_type_void;
            break;
    }
}

void va_arg_to_fcitx_arg(char c, FcitxAddonFunctionArgument* arg, va_list ap)
{
#define CHAR_TO_FIELD(CHAR, FIELD, C_TYPE) \
    case CHAR: \
        arg->FIELD = va_arg(ap, C_TYPE); \
        break;
    switch(c) {
        CHAR_TO_FIELD('a', a, void*)
        CHAR_TO_FIELD('y', s8, int)
        CHAR_TO_FIELD('n', s16, int)
        CHAR_TO_FIELD('i', s32, int32_t)
        CHAR_TO_FIELD('x', s64, int64_t)
        CHAR_TO_FIELD('z', u8, int)
        CHAR_TO_FIELD('q', u16, int)
        CHAR_TO_FIELD('u', u32, uint32_t)
        CHAR_TO_FIELD('t', u64, uint64_t)
        CHAR_TO_FIELD('f', f, double)
        CHAR_TO_FIELD('d', d, double)
        case 'v':
            break;
    }
}

FCITX_EXPORT_API
void fcitx_addon_manager_invoke(FcitxAddonManager* manager, const char* addonName, const char* functionName, void* retVal, ...)
{
    FcitxAddon* addon;
    if (!fcitx_dict_lookup_by_str(manager->addons, addonName, &addon)) {
        return;
    }

    if (!addon->inst.functions) {
        return;
    }

    FcitxAddonFunctionEntry* entry;
    if (!fcitx_dict_lookup_by_str(addon->inst.functions, functionName, &entry)) {
        return;
    }

    // bad addon!
    if (!entry->function || !entry->signature) {
        return;
    }

    size_t siglen = strlen(entry->signature);
    if (siglen == 0 || siglen > FCITX_ADDON_FUNCTION_MAX_ARG + 1) {
        return;
    }
    // validate signature
    for (size_t i = 0; i < siglen; i++) {
        if (entry->signature[i] == 'v' && i != 0) {
            return;
        }

        if (!strchr("aynixzqutfdv", entry->signature[i])) {
            return;
        }
    }

    va_list ap;

    ffi_type *ffi_types[FCITX_ADDON_FUNCTION_MAX_ARG + 1];
    void* ffi_args[FCITX_ADDON_FUNCTION_MAX_ARG + 1];
    FcitxAddonFunctionArgument fargs[FCITX_ADDON_FUNCTION_MAX_ARG + 1];
    // signature 0 is return value, but we also need a extra value

    va_start(ap, retVal);
    for (size_t i = 1; i < siglen; i++) {
        va_arg_to_fcitx_arg(entry->signature[i], &fargs[i], ap);
        fcitx_arg_to_ffi_args(entry->signature[i], &fargs[i], &ffi_types[i], &ffi_args[i]);
    }
    va_end(ap);

    // self
    ffi_args[0] = &addon->inst.data;
    ffi_types[0] = &ffi_type_pointer;

    ffi_type* ffi_rettype = NULL;

    fcitx_arg_to_ffi_args(entry->signature[0], NULL, &ffi_rettype, NULL);

    ffi_cif cif;
    if (FFI_OK != ffi_prep_cif(&cif, FFI_DEFAULT_ABI, siglen, ffi_rettype, ffi_types)) {
        return;
    }
    ffi_call(&cif, entry->function, retVal, ffi_args);
}
