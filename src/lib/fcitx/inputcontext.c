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

#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "inputcontext-internal.h"
#include <uuid/uuid.h>

/**
 * relation between group and ic
 * when a group is free'd, move all ic to global group
 */
struct _FcitxInputContextFocusGroup
{
    FcitxListHead inputContexts;
    FcitxInputContext* focus;
    FcitxInputContextManager* manager;
    FcitxListHead list;
};

typedef struct _FcitxInputContextManager
{
    int refcount;
    uint32_t icid;
    FcitxInputContext* freeList;
    FcitxInputContext* ics;
    FcitxInputContextFocusGroup* globalGroup;
    FcitxListHead groups;
} FcitxInputContextManager;

static FcitxInputContextFocusGroup* _fcitx_input_context_manager_create_focus_group(FcitxInputContextManager* manager);
static void _fcitx_input_context_set_focus_group(FcitxInputContext* ic, FcitxInputContextFocusGroup* group);
static void _fcitx_input_context_focus_in(FcitxInputContext* ic);
static void _fcitx_input_context_focus_out(FcitxInputContext* ic);
static void _fcitx_input_context_destroy(FcitxInputContext* ic);

FCITX_EXPORT_API
FcitxInputContextManager* fcitx_input_context_manager_new()
{
    FcitxInputContextManager* manager = fcitx_utils_new(FcitxInputContextManager);
    manager->globalGroup = _fcitx_input_context_manager_create_focus_group(manager);
    fcitx_list_init(&manager->groups);
    return fcitx_input_context_manager_ref(manager);
}

void fcitx_input_context_manager_free(FcitxInputContextManager* manager)
{
    fcitx_list_entry_foreach_safe(group, FcitxInputContextFocusGroup, &manager->groups, list) {
        fcitx_input_context_focus_group_free(group);
    }

    fcitx_input_context_focus_group_free(manager->globalGroup);

    while (manager->ics) {
        fcitx_input_context_destroy(manager->ics);
    }

    while (manager->freeList) {
        FcitxInputContext* ic = manager->freeList;
        // TODO: mind need to free more?
        manager->freeList = manager->freeList->hh.next;
        free(ic);
    }
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputContextManager, fcitx_input_context_manager);

FcitxInputContextFocusGroup* _fcitx_input_context_manager_create_focus_group(FcitxInputContextManager* manager)
{
    FcitxInputContextFocusGroup* group = fcitx_utils_new(FcitxInputContextFocusGroup);
    fcitx_list_init(&group->inputContexts);
    group->manager = manager;

    return group;

}
FCITX_EXPORT_API
FcitxInputContextFocusGroup* fcitx_input_context_manager_create_focus_group(FcitxInputContextManager* manager)
{
    FcitxInputContextFocusGroup* group = _fcitx_input_context_manager_create_focus_group(manager);
    fcitx_list_append(&group->list, &manager->groups);

    return group;

}

FCITX_EXPORT_API
void fcitx_input_context_focus_group_free(FcitxInputContextFocusGroup* group)
{
    if (group != group->manager->globalGroup) {
        fcitx_list_remove(&group->list);
    }

    fcitx_list_entry_foreach_safe(entry, FcitxInputContext, &group->inputContexts, list) {
        _fcitx_input_context_set_focus_group(entry, NULL);
    }

    free(group);
}

void _fcitx_input_context_set_focus_group(FcitxInputContext* ic, FcitxInputContextFocusGroup* group)
{
    if (ic->group == group) {
        return;
    }

    if (ic->group) {
        fcitx_list_remove(&ic->list);
    }

    if (group) {
        fcitx_list_append(&ic->list, &group->inputContexts);
    }
    ic->group = group;
    if (ic->focused) {
        _fcitx_input_context_focus_out(ic);
    }
}

FCITX_EXPORT_API
void fcitx_input_context_set_focus_group(FcitxInputContext* ic, FcitxInputContextFocusGroupType type, FcitxInputContextFocusGroup* group)
{
    if ((type == FICFG_Local && group == NULL)
     || (type != FICFG_Local && group != NULL)) {
        return;
    }

    if (type == FICFG_Global) {
        group = ic->manager->globalGroup;
    }

    _fcitx_input_context_set_focus_group(ic, group);
}


FCITX_EXPORT_API
FcitxInputContext* fcitx_input_context_manager_create_ic(FcitxInputContextManager* manager,
                                                         FcitxInputContextFillDataCallback callback,
                                                         void* userData)
{
    FcitxInputContext* ic = NULL;
    if (manager->freeList) {
        ic = manager->freeList;
        manager->freeList = ic->hh.next;
    } else {
        uint32_t newid;
        while ((newid = ++manager->icid) == 0);
        HASH_FIND(hh, manager->ics, &newid, sizeof(uint32_t), ic);
        if (ic) {
            return NULL;
        }
        ic = fcitx_utils_new(FcitxInputContext);
        ic->id = newid;
        ic->manager = manager;
    }

    uuid_generate(ic->uuid);

    HASH_ADD(hh, manager->ics, id, sizeof(uint32_t), ic);

    if (callback) {
        callback(ic, userData);
    }

    return ic;
}

void _fcitx_input_context_group_set_focus(FcitxInputContextFocusGroup* group, FcitxInputContext* ic)
{
    FcitxInputContext* focus = group->focus;
    group->focus = ic;
    if (focus) {
        _fcitx_input_context_focus_out(focus);
    }

    if (ic) {
        _fcitx_input_context_focus_in(ic);
    }
}

FCITX_EXPORT_API
void fcitx_input_context_manager_focus_in(FcitxInputContextManager* manager, uint32_t id)
{
    FcitxInputContext* ic = fcitx_input_context_manager_get_ic(manager, id);
    if (!ic) {
        return;
    }

    if (ic->group == manager->globalGroup) {
        fcitx_list_entry_foreach(group, FcitxInputContextFocusGroup, &manager->groups, list) {
            _fcitx_input_context_group_set_focus(group, NULL);
        }

        _fcitx_input_context_group_set_focus(manager->globalGroup, ic);
    } else if (ic->group) {
        _fcitx_input_context_group_set_focus(manager->globalGroup, NULL);
        _fcitx_input_context_group_set_focus(ic->group, ic);
    } else {
        if (!ic->focused) {
            _fcitx_input_context_focus_in(ic);
        }
    }
}

FCITX_EXPORT_API
void fcitx_input_context_manager_focus_out(FcitxInputContextManager* manager, uint32_t id)
{
    FcitxInputContext* ic = fcitx_input_context_manager_get_ic(manager, id);
    if (!ic) {
        return;
    }

    if (ic->group) {
        if (ic->group->focus == ic) {
            _fcitx_input_context_group_set_focus(ic->group, NULL);
        }
    } else {
        if (ic->focused) {
            _fcitx_input_context_focus_out(ic);
        }
    }
}

FCITX_EXPORT_API
FcitxInputContext* fcitx_input_context_manager_get_ic(FcitxInputContextManager* manager, uint32_t id)
{
    FcitxInputContext* ic = NULL;
    HASH_FIND(hh, manager->ics, &id, sizeof(uint32_t), ic);
    return ic;
}

FCITX_EXPORT_API
void fcitx_input_context_destroy(FcitxInputContext* inputContext)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (inputContext != fcitx_input_context_manager_get_ic(manager, inputContext->id)) {
        return;
    }

    fcitx_input_context_manager_focus_out(manager, inputContext->id);
    _fcitx_input_context_destroy(inputContext);
    //inputContext->destroyNotify(inputContext->userData);
    HASH_DEL(manager->ics, inputContext);

    inputContext->hh.next = manager->freeList;
    manager->freeList = inputContext;
}

FCITX_EXPORT_API
uint32_t fcitx_input_context_get_id(FcitxInputContext* inputContext)
{
    return inputContext->id;
}

FCITX_EXPORT_API
void fcitx_input_context_reset(FcitxInputContext* inputContext)
{

}

FCITX_EXPORT_API
bool fcitx_input_context_process_event(FcitxInputContext* inputContext, FcitxKeyEvent* event)
{

}

FCITX_EXPORT_API
bool fcitx_input_context_is_focused(FcitxInputContext* inputContext)
{
    return inputContext->focused;
}

void _fcitx_input_context_focus_in(FcitxInputContext* ic)
{
    ic->focused = true;
}

void _fcitx_input_context_focus_out(FcitxInputContext* ic)
{
    ic->focused = false;
}

void _fcitx_input_context_destroy(FcitxInputContext* ic)
{

}
