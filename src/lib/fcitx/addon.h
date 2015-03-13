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

typedef void* (*FcitxAddonInitFunc)(FcitxAddonManager* manager, const FcitxAddonConfig* config);
typedef void (*FcitxAddonDestroyFunc)(void* addon);
typedef void (*FcitxAddonReloadConfig)(void* addon);
typedef FcitxDict* (*FcitxAddonRegisterCabllack)();
typedef struct _FcitxStaticAddon {
    char* name;
    void* entry;
} FcitxStaticAddon;

typedef struct _FcitxAddonFunctionEntry
{
    FcitxCallback function;
    char signature[];
} FcitxAddonFunctionEntry;

typedef union _FcitxAddonFunctionArgument
{
    int8_t s8;
    int16_t s16;
    int32_t s32;
    int64_t s64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    float f;
    double d;
    void* a;
} FcitxAddonFunctionArgument;

#define FCITX_STATIC_ADDON(NAME, ENTRY) (FcitxStaticAddon){.name = (NAME), .entry = (ENTRY)}
#define FCITX_STATIC_ADDON_END() FCITX_STATIC_ADDON(NULL, NULL)

typedef struct _FcitxAddonAPICommon
{
    FcitxAddonInitFunc init;
    FcitxAddonDestroyFunc destroy;
    FcitxAddonReloadConfig reloadConfig;
    FcitxAddonRegisterCabllack registerCallback;
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
    FcitxDict* functions;
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
    void* padding1;
    void* padding2;
    void* padding3;
    void* padding4;
    void* padding5;
    void* padding6;
} FcitxAddonResolver;

#define FCITX_ADDON_FUNCTION_MAX_ARG 20

FcitxAddonManager* fcitx_addon_manager_new(FcitxStandardPath* standardPath);
FcitxAddonManager* fcitx_addon_manager_ref(FcitxAddonManager* manager);
void fcitx_addon_manager_unref(FcitxAddonManager* manager);
void fcitx_addon_manager_set_property(FcitxAddonManager* manager, const char* name, void* data);
void* fcitx_addon_manager_get_property(FcitxAddonManager* manager, const char* name);
void fcitx_addon_manager_set_override(FcitxAddonManager* manager, const char* enabled, const char* disabled);
void fcitx_addon_manager_register_default_resolver(FcitxAddonManager* mananger, FcitxStaticAddon* staticAddon);
void fcitx_addon_manager_load(FcitxAddonManager* manager);
void fcitx_addon_manager_unload(FcitxAddonManager* manager);
void fcitx_addon_manager_invoke(FcitxAddonManager* manager, const char* addonName, const char* functionName, void* retVal, ...);
FcitxStandardPath* fcitx_addon_manager_get_standard_path(FcitxAddonManager* manager);

#endif // __FCITX_ADDON_H__
