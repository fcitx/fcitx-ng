/*
 * Copyright (C) 2002~2005 by Yuking
 * yuking_net@sohu.com
 * Copyright (C) 2012~2015 by CSSlayer
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

#include <stdarg.h>

#include "ime.h"
#include "ime-internal.h"
#include "addon-internal.h"
#include "fcitx-utils/macro-internal.h"
#include "input-method-metadata.h"
#include "input-method-list.h"

typedef struct _FcitxKeyboardLayoutInfo
{
    char* layout;
    char* variant;
} FcitxKeyboardLayoutInfo;

typedef struct _FcitxInputMethodGroup
{
    FcitxKeyboardLayoutInfo layoutInfo;
    FcitxPtrArray* items;
} FcitxInputMethodGroup;

struct _FcitxInputMethodItem
{
    FcitxKeyboardLayoutInfo layoutInfo;
    char* name;
    size_t index;
    FcitxInputMethodGroup* group;
    size_t keyLength;
    char key[];
};

struct _FcitxInputMethodManager
{
    int refcount;
    FcitxPtrArray* groups;
    FcitxDispatchEventCallback callback;
    FcitxDestroyNotify destroyNotify;
    void* userData;
    FcitxAddonManager* addonManager;
    FcitxDict* ims;
};

void fcitx_input_method_group_free(FcitxInputMethodGroup* group)
{
    free(group->layoutInfo.layout);
    free(group->layoutInfo.variant);
    fcitx_ptr_array_free(group->items);
    free(group);
}

void fcitx_input_method_item_free(FcitxInputMethodItem* item)
{
    free(item);
}

FCITX_EXPORT_API
FcitxInputMethodManager* fcitx_input_method_manager_new(FcitxAddonManager* addonManager)
{
    FcitxInputMethodManager* self = fcitx_utils_new(FcitxInputMethodManager);
    self->addonManager = addonManager;
    self->groups = fcitx_ptr_array_new((FcitxDestroyNotify) fcitx_input_method_group_free);
    self->ims = fcitx_dict_new(NULL);
    return fcitx_input_method_manager_ref(self);
}


void fcitx_input_method_manager_free(FcitxInputMethodManager* self)
{
    fcitx_dict_free(self->ims);
    fcitx_ptr_array_free(self->groups);

    if (self->destroyNotify) {
        self->destroyNotify(self->userData);
    }
    free(self);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputMethodManager, fcitx_input_method_manager);


FCITX_EXPORT_API
int fcitx_input_method_manager_create_group(FcitxInputMethodManager* manager, ...)
{
    va_list va;
    va_start(va, manager);
    const char *layout = NULL, *variant = NULL;
    char *name;
    while ((name = va_arg(va, char*))) {
        if (strcmp(name, "layout") == 0) {
            layout = va_arg(va, char*);
        } else if (strcmp(name, "variant") == 0) {
            variant = va_arg(va, char*);
        }
    }

    va_end(va);

    if (!layout) {
        return -1;
    }

    FcitxInputMethodGroup* group = fcitx_utils_new(FcitxInputMethodGroup);
    group->layoutInfo.layout = fcitx_utils_strdup(layout);
    group->layoutInfo.variant = fcitx_utils_strdup(variant);
    group->items = fcitx_ptr_array_new((FcitxDestroyNotify) fcitx_input_method_item_free);
    fcitx_ptr_array_append(manager->groups, group);
    return fcitx_ptr_array_size(manager->groups) - 1;
}

FCITX_EXPORT_API
void fcitx_input_method_manager_reset_group(FcitxInputMethodManager* manager)
{
    fcitx_ptr_array_clear(manager->groups);
}

size_t fcitx_input_method_manager_group_count(FcitxInputMethodManager* manager)
{
    return fcitx_ptr_array_size(manager->groups);
}

FCITX_EXPORT_API
bool fcitx_input_method_manager_is_group_empty(FcitxInputMethodManager* manager, int groupId)
{
    FcitxInputMethodGroup* group = fcitx_ptr_array_index(manager->groups, groupId, FcitxInputMethodGroup*);

    return fcitx_ptr_array_size(group->items) == 0;
}


FCITX_EXPORT_API
void fcitx_input_method_manager_set_input_method_list(FcitxInputMethodManager* manager, int groupId, const char* const* ims)
{
    if (groupId < 0 || groupId > (ssize_t) fcitx_ptr_array_size(manager->groups)) {
        return;
    }

    FcitxInputMethodGroup* group = fcitx_ptr_array_index(manager->groups, groupId, FcitxInputMethodGroup*);

    FcitxInputMethodItem item;
    memset(&item, 0, sizeof(item));

    for (size_t i = 0; ims[i]; i ++) {
        FcitxStringList* list = fcitx_utils_string_split(ims[i], ",");
        utarray_foreach(itemString, list, char*) {
            char* colon = strchr(*itemString, ':');
            if (!colon) {
                continue;
            }
            *colon = '\0';
            colon++;

            if (strcmp(*itemString, "name") == 0) {
                item.name = colon;
            } else if (strcmp(*itemString, "layout") == 0) {
                item.layoutInfo.layout = colon;
            } else if (strcmp(*itemString, "variant") == 0) {
                item.layoutInfo.variant = colon;
            }
        }

        // validate data
        if (item.name) {
            size_t offset[] = {
                offsetof(FcitxInputMethodItem, name),
                offsetof(FcitxInputMethodItem, layoutInfo.layout),
                offsetof(FcitxInputMethodItem, layoutInfo.variant),
            };

            size_t length[FCITX_ARRAY_SIZE(offset)];
            size_t sumOfLength = 0;

            for (size_t i = 0; i < FCITX_ARRAY_SIZE(offset); i++) {
                char* str = *(char**)(((char*)&item) + offset[i]);
                length[i] = str ? strlen(str) : 0;
                sumOfLength += length[i] + 1;
            }

            FcitxInputMethodItem* newItem = fcitx_utils_malloc0(sizeof(FcitxInputMethodItem) + sumOfLength);
            newItem->keyLength = sumOfLength;
            sumOfLength = 0;
            for (size_t i = 0; i < FCITX_ARRAY_SIZE(offset); i++) {
                char* str = *(char**)(((char*)&item) + offset[i]);
                if (str) {
                    memcpy(&newItem->key[sumOfLength], str, length[i] + 1);
                }
                *(char**)(((char*)newItem) + offset[i]) = str ? &newItem->key[sumOfLength] : NULL;
                sumOfLength += length[i] + 1;
            }
            newItem->index = fcitx_ptr_array_size(group->items);
            newItem->group = group;
            fcitx_ptr_array_append(group->items, newItem);
        }

        fcitx_utils_string_list_free(list);
    }
}

FCITX_EXPORT_API
FcitxInputMethodItem* fcitx_input_method_manager_get_group_item(FcitxInputMethodManager* manager, int groupId, size_t index)
{
    if (groupId < 0 || groupId > (ssize_t) fcitx_ptr_array_size(manager->groups)) {
        return NULL;
    }

    FcitxInputMethodGroup* group = fcitx_ptr_array_index(manager->groups, groupId, FcitxInputMethodGroup*);

    return fcitx_ptr_array_index(group->items, index, FcitxInputMethodItem*);
}

FCITX_EXPORT_API
size_t fcitx_input_method_manager_get_group_size(FcitxInputMethodManager* manager, int groupId)
{
    if (groupId < 0 || groupId > (ssize_t) fcitx_ptr_array_size(manager->groups)) {
        return 0;
    }

    FcitxInputMethodGroup* group = fcitx_ptr_array_index(manager->groups, groupId, FcitxInputMethodGroup*);

    return fcitx_ptr_array_size(group->items);
}

FCITX_EXPORT_API
void fcitx_input_method_manager_set_event_dispatcher(FcitxInputMethodManager* manager, FcitxDispatchEventCallback callback, FcitxDestroyNotify destroyNotify, void* userData)
{
    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }

    manager->callback = callback;
    manager->destroyNotify = destroyNotify;
    manager->userData = userData;
}

void fcitx_input_method_item_get_property(FcitxInputMethodItem* item, ...)
{
    va_list va;
    va_start(va, item);
    FcitxInputMethodItemProperty property;
    while ((property = va_arg(va, FcitxInputMethodItemProperty))) {
        switch (property) {
            case FIMIP_Name: {
                char** name = va_arg(va, char**);
                *name = item->name;
            }
            break;
            case FIMIP_Layout: {
                char** name = va_arg(va, char**);
                *name = item->layoutInfo.layout;
            }
            break;
            case FIMIP_Variant: {
                char** name = va_arg(va, char**);
                *name = item->layoutInfo.variant;
            }
            break;
            default:
                break;
        }
    }
    va_end(va);
}

FCITX_EXPORT_API
void fcitx_input_method_manager_save_configuration(FcitxInputMethodManager* manager, FILE* fp)
{
    // fcitx_configuration_new();
}

FCITX_EXPORT_API
FcitxInputMethod* fcitx_input_method_manager_get(FcitxInputMethodManager* manager, FcitxInputMethodItem* item)
{
    FcitxInputMethod* inputMethod;
    if (fcitx_dict_lookup(manager->ims, item->key, item->keyLength, &inputMethod)) {
        return inputMethod;
    }

    return NULL;
}

FCITX_EXPORT_API
void fcitx_input_method_manager_load_metadata(FcitxInputMethodManager* manager)
{
    FcitxInputMethodList* list = fcitx_input_method_list_new();
    for (size_t i = 0; i < fcitx_ptr_array_size(manager->addonManager->ims); i++) {
        FcitxAddon* addon = fcitx_ptr_array_index(manager->addonManager->ims, i, FcitxAddon*);
        FcitxAddonAPIInputMethod* api = addon->inst.api;
        FcitxConfiguration* metadataConfig = api->listInputMethod(addon->inst.data);

        fcitx_input_method_list_load(list, metadataConfig);
        for (uint32_t i = 0; i < list->inputMethods.inputMethods->len; i++) {
            char* function = *(char**) list->inputMethods.inputMethods->data[i];
            FcitxConfiguration* subConfig = fcitx_configuration_get(metadataConfig, function, false);
            if (!subConfig) {
                continue;
            }

            FcitxInputMethodMetadata* metadata = fcitx_input_method_metadata_new();
            fcitx_input_method_metadata_load(metadata, subConfig);

            if (metadata->iM.uniqueName[0]) {
                
            }

            fcitx_input_method_metadata_free(metadata);
        }

        fcitx_configuration_unref(metadataConfig);
    }
    fcitx_input_method_list_free(list);
}
