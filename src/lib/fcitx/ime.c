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

typedef struct _FcitxInputMethod
{
    char* uniqueName;
    char* name;
    char* iconName;
    int priority;
    char* langCode;
    void* imclass;
    bool (*handleEvent)(void*, const FcitxIMEvent*);
} FcitxInputMethod;

typedef struct _FcitxKeyboardLayoutInfo
{
    char* layout;
    char* variant;
} FcitxKeyboardLayoutInfo;

typedef struct _FcitxInputMethodGroup
{
    FcitxKeyboardLayoutInfo layoutInfo;
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
    FcitxInputMethodGroup defaultGroup;
    FcitxPtrArray* groups;
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
    free(group);
}

FCITX_EXPORT_API
FcitxInputMethodManager* fcitx_input_method_manager_new()
{
    FcitxInputMethodManager* manager = fcitx_utils_new(FcitxInputMethodManager);
    manager->ims = fcitx_dict_new((FcitxDestroyNotify) fcitx_input_method_free);
    manager->nextGroupId = 1;
    manager->groups = fcitx_ptr_array_new((FcitxDestroyNotify) fcitx_input_method_group_free);
    return fcitx_input_method_manager_ref(manager);
}


void fcitx_input_method_manager_free(FcitxInputMethodManager* manager)
{
    fcitx_ptr_array_free(manager->groups);
    fcitx_dict_free(manager->ims);
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputMethodManager, fcitx_input_method_manager);

FCITX_EXPORT_API
bool fcitx_input_method_manager_register(FcitxInputMethodManager* manager,
                                         void* imclass,
                                         const char* uniqueName,
                                         const char* name,
                                         const char* iconName,
                                         bool (*handleEvent)(void*, const FcitxIMEvent*),
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

int fcitx_input_method_manager_create_group(FcitxInputMethodManager* manager, ...)
{
    va_list va;
    va_start(va, manager);
    const char *layout = NULL, *variant = NULL;
    char *name;
    while ((name = va_arg(va, char*))) {
        if (strcmp(layout, "layout") == 0) {
            layout = va_arg(va, char*);
        } else if (strcmp(layout, "layout") == 0) {
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
    fcitx_ptr_array_append(manager->groups, group);
    return fcitx_ptr_array_size(manager->groups);
}

void fcitx_input_method_manager_set_input_method_list(FcitxInputMethodManager* manager, int groupId, const char* const* ims)
{
    if (groupId < 0 || groupId > (ssize_t) fcitx_ptr_array_size(manager->groups)) {
        return;
    }

    FcitxInputMethodGroup* group = groupId == 0 ? (&manager->defaultGroup) : fcitx_ptr_array_index(manager->groups, groupId - 1, FcitxInputMethodGroup*);

    FcitxInputMethodItem item;

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

        if (item.name) {
        }

        fcitx_utils_string_list_free(list);
    }
}
