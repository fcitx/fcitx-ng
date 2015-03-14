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
#include "fcitx-utils/macro-internal.h"

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

typedef struct _FcitxInputMethodItem
{
    FcitxKeyboardLayoutInfo layoutInfo;
    char* name;
} FcitxInputMethodItem;

struct _FcitxInputMethodManager
{
    int refcount;
    FcitxDict* ims;
    int nextGroupId;
    FcitxPtrArray* groups;
    FcitxDispatchEventCallback callback;
    FcitxDestroyNotify destroyNotify;
    void* userData;
};

void fcitx_input_method_free(FcitxInputMethod* im)
{
    free(im->uniqueName);
    free(im->name);
    free(im->iconName);
    free(im->langCode);
    free(im);
}

void fcitx_input_method_group_free(FcitxInputMethodGroup* group)
{
    free(group->layoutInfo.layout);
    free(group->layoutInfo.variant);
    fcitx_ptr_array_free(group->items);
    free(group);
}

void fcitx_input_method_item_free(FcitxInputMethodItem* item)
{
    free(item->name);
    free(item->layoutInfo.layout);
    free(item->layoutInfo.variant);
    free(item);
}

FCITX_EXPORT_API
FcitxInputMethodManager* fcitx_input_method_manager_new()
{
    FcitxInputMethodManager* manager = fcitx_utils_new(FcitxInputMethodManager);
    manager->ims = fcitx_dict_new((FcitxDestroyNotify) fcitx_input_method_free);
    manager->nextGroupId = 1;
    manager->groups = fcitx_ptr_array_new((FcitxDestroyNotify) fcitx_input_method_group_free);
    fcitx_input_method_manager_create_group(manager, "layout", "us", NULL);
    return fcitx_input_method_manager_ref(manager);
}


void fcitx_input_method_manager_free(FcitxInputMethodManager* manager)
{
    fcitx_ptr_array_free(manager->groups);
    fcitx_dict_free(manager->ims);

    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputMethodManager, fcitx_input_method_manager);

FCITX_EXPORT_API
bool fcitx_input_method_manager_register(FcitxInputMethodManager* manager,
                                         void* imclass,
                                         const char* uniqueName,
                                         const char* name,
                                         const char* iconName,
                                         FcitxDispatchEventCallback handleEvent,
                                         int priority, const char* langCode)
{
    FcitxInputMethod* im = NULL;
    if (fcitx_dict_lookup_by_str(manager->ims, uniqueName, &im)) {
        if (im->imclass) {
            return false;
        } else {
            im->imclass = imclass;
            im->handleEvent = handleEvent;
        }
    } else {
        im = fcitx_utils_new(FcitxInputMethod);
        im->uniqueName = fcitx_utils_strdup(uniqueName);
        im->name = fcitx_utils_strdup(name);
        im->iconName = fcitx_utils_strdup(iconName);
        im->langCode = fcitx_utils_strdup(langCode);
        im->priority = priority;
        im->imclass = imclass;
        im->handleEvent = handleEvent;

        fcitx_dict_insert_by_str(manager->ims, uniqueName, im, false);
    }

    return true;
}

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
    while (fcitx_ptr_array_size(manager->groups) > 1) {
        fcitx_ptr_array_pop(manager->groups, NULL);
    }

    FcitxInputMethodGroup* group = fcitx_ptr_array_index(manager->groups, 0, FcitxInputMethodGroup*);
    fcitx_ptr_array_clear(group->items);
    fcitx_utils_string_swap(&group->layoutInfo.layout, "us");
    fcitx_utils_string_swap(&group->layoutInfo.variant, NULL);
}

size_t fcitx_input_method_manager_group_count(FcitxInputMethodManager* manager)
{
    return fcitx_ptr_array_size(manager->groups);
}

FCITX_EXPORT_API
bool fcitx_input_method_manager_is_group_empty(FcitxInputMethodManager* manager, int group)
{
    FcitxInputMethodGroup* group = fcitx_ptr_array_index(manager->groups, group, FcitxInputMethodGroup*);

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
            FcitxInputMethodItem* newItem = fcitx_utils_new(FcitxInputMethodItem);
            newItem->name = fcitx_utils_strdup(item.name);
            newItem->layoutInfo.layout = fcitx_utils_strdup(item.layoutInfo.layout);
            newItem->layoutInfo.variant = fcitx_utils_strdup(item.layoutInfo.variant);
            fcitx_ptr_array_append(group->items, newItem);
        }

        fcitx_utils_string_list_free(list);
    }
}

void fcitx_input_method_manager_set_event_dispatcher(FcitxInputMethodManager* manager, FcitxDispatchEventCallback callback, FcitxDestroyNotify destroyNotify, void* userData)
{
    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }

    manager->callback = callback;
    manager->destroyNotify = destroyNotify;
    manager->userData = userData;
}
