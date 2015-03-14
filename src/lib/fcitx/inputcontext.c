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
    FcitxDispatchEventCallback callback;
    FcitxDestroyNotify destroyNotify;
    void* userData;
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
        // mind need to free more?
        manager->freeList = manager->freeList->hh.next;
        free(ic);
    }

    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }

    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputContextManager, fcitx_input_context_manager);

void fcitx_input_context_manager_set_event_dispatcher(FcitxInputContextManager* manager, FcitxDispatchEventCallback callback, FcitxDestroyNotify destroyNotify, void* userData)
{
    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }

    manager->callback = callback;
    manager->destroyNotify = destroyNotify;
    manager->userData = userData;
}


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
FcitxInputContext* fcitx_input_context_manager_create_ic(FcitxInputContextManager* manager, uint32_t frontend)
{
    FcitxInputContext* ic = NULL;
    if (manager->freeList) {
        ic = manager->freeList;
        uint32_t oldId = ic->id;
        memset(ic, 0, sizeof(*ic));
        ic->id = oldId;
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
    }
    ic->manager = manager;
    ic->frontend = frontend;

    uuid_generate(ic->uuid);

    HASH_ADD(hh, manager->ics, id, sizeof(uint32_t), ic);

    if (manager->callback) {
        FcitxInputContextEvent event;
        event.type = ET_InputContextCreated;
        event.id = ic->id;
        event.inputContext = ic;
        manager->callback(manager->userData, (FcitxEvent*) &event);
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
void fcitx_input_context_focus_in(FcitxInputContext* inputContext)
{
    FcitxInputContextManager* manager = inputContext->manager;

    if (inputContext->group == manager->globalGroup) {
        fcitx_list_entry_foreach(group, FcitxInputContextFocusGroup, &manager->groups, list) {
            _fcitx_input_context_group_set_focus(group, NULL);
        }

        _fcitx_input_context_group_set_focus(manager->globalGroup, inputContext);
    } else if (inputContext->group) {
        _fcitx_input_context_group_set_focus(manager->globalGroup, NULL);
        _fcitx_input_context_group_set_focus(inputContext->group, inputContext);
    } else {
        if (!inputContext->focused) {
            _fcitx_input_context_focus_in(inputContext);
        }
    }
}

FCITX_EXPORT_API
void fcitx_input_context_focus_out(FcitxInputContext* inputContext)
{
    if (inputContext->group) {
        if (inputContext->group->focus == inputContext) {
            _fcitx_input_context_group_set_focus(inputContext->group, NULL);
        }
    } else {
        if (inputContext->focused) {
            _fcitx_input_context_focus_out(inputContext);
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

    fcitx_input_context_focus_out(inputContext);
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
void fcitx_input_context_get_uuid(FcitxInputContext* inputContext, uint8_t* uuid)
{
    memcpy(uuid, inputContext->uuid, sizeof(inputContext->uuid));
}

FCITX_EXPORT_API
uint32_t fcitx_input_context_get_capability_flags(FcitxInputContext* inputContext)
{
    return inputContext->flags;
}

FCITX_EXPORT_API
void fcitx_input_context_set_capability_flags(FcitxInputContext* inputContext, uint32_t flags)
{
    if (inputContext->flags != flags) {
        inputContext->flags = flags;
        FcitxInputContextManager* manager = inputContext->manager;
        if (manager->callback) {
            FcitxInputContextEvent event;
            event.type = ET_InputContextCapabilityChanged;
            event.id = inputContext->id;
            event.inputContext = inputContext;
            manager->callback(manager->userData, (FcitxEvent*) &event);
        }
    }
}

FCITX_EXPORT_API
void fcitx_input_context_set_cursor_rect(FcitxInputContext* inputContext, FcitxRect rect)
{
    if (inputContext->rect.x1 != rect.x1 ||
        inputContext->rect.x2 != rect.x2 ||
        inputContext->rect.y1 != rect.y1 ||
        inputContext->rect.y2 != rect.y2) {
        inputContext->rect = rect;
        FcitxInputContextManager* manager = inputContext->manager;
        if (manager->callback) {
            FcitxInputContextEvent event;
            event.type = ET_InputContextCursorRectChanged;
            event.id = inputContext->id;
            event.inputContext = inputContext;
            manager->callback(manager->userData, (FcitxEvent*) &event);
        }
    }
}

FCITX_EXPORT_API
void fcitx_input_context_reset(FcitxInputContext* inputContext)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (manager->callback) {
        FcitxInputContextEvent event;
        event.type = ET_InputContextReset;
        event.id = inputContext->id;
        event.inputContext = inputContext;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

FCITX_EXPORT_API
bool fcitx_input_context_process_key_event(FcitxInputContext* inputContext, FcitxKeyEvent* key)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (manager->callback) {
        FcitxInputContextKeyEvent event;
        event.type = ET_InputContextKeyEvent;
        event.id = inputContext->id;
        event.inputContext = inputContext;
        event.detail = *key;
        event.detail.key = fcitx_key_normalize(event.detail.rawKey);
        return manager->callback(manager->userData, (FcitxEvent*) &event);
    }

    return false;
}

FCITX_EXPORT_API
bool fcitx_input_context_is_focused(FcitxInputContext* inputContext)
{
    return inputContext->focused;
}

FCITX_EXPORT_API
void fcitx_input_context_commit_string(FcitxInputContext* inputContext, const char* commitString)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (manager->callback) {
        FcitxInputContextCommitStringEvent event;
        event.type = ET_InputContextCommitString;
        event.id = inputContext->id;
        event.inputContext = inputContext;
        event.commitString = commitString;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

FCITX_EXPORT_API
bool fcitx_input_context_get_surrounding_text(FcitxInputContext* inputContext, const char** str, unsigned int* cursor, unsigned int* anchor)
{
    if (!(inputContext->flags & CAPABILITY_SURROUNDING_TEXT)) {
        return false;
    }

    if (!inputContext->surroundingText) {
        return false;
    }

    if (str) {
        *str = inputContext->surroundingText;
    }

    if (cursor) {
        *cursor = inputContext->cursor;
    }

    if (anchor) {
        *anchor = inputContext->anchor;
    }

    return true;
}

FCITX_EXPORT_API
void fcitx_input_context_set_surrounding_text(FcitxInputContext* inputContext, const char* str, unsigned int cursor, unsigned int anchor)
{
    if (!(inputContext->flags & CAPABILITY_SURROUNDING_TEXT)) {
        return;
    }

    if (!inputContext->surroundingText || (str && strcmp(inputContext->surroundingText, str) != 0) || cursor != inputContext->cursor || anchor != inputContext->anchor) {
        if (str) {
            fcitx_utils_string_swap(&inputContext->surroundingText, str);
        }
        inputContext->cursor = cursor;
        inputContext->anchor = anchor;

        FcitxInputContextManager* manager = inputContext->manager;
        if (manager->callback) {
            FcitxInputContextEvent event;
            event.type = ET_InputContextFocusIn;
            event.id = inputContext->id;
            event.inputContext = inputContext;
            manager->callback(manager->userData, (FcitxEvent*) &event);
        }
    }
}

void fcitx_input_context_delete_surrounding_text(FcitxInputContext* inputContext, int offset, unsigned int length)
{
    /*
     * do the real deletion here, and client might update it, but input method itself
     * would expect a up to date value after this call.
     *
     * Make their life easier.
     */
    if (inputContext->surroundingText) {
        int cursor_pos = inputContext->cursor + offset;
        size_t len = fcitx_utf8_strlen (inputContext->surroundingText);
        if (cursor_pos >= 0 && len - cursor_pos >= length) {
            /*
            * the original size must be larger, so we can do in-place copy here
            * without alloc new string
            */
            char* start = fcitx_utf8_get_nth_char(inputContext->surroundingText, cursor_pos);
            char* end = fcitx_utf8_get_nth_char(start, length);

            int copylen = strlen(end);

            memmove (start, end, sizeof(char) * copylen);
            start[copylen] = 0;
            inputContext->cursor = cursor_pos;
        } else {
            inputContext->surroundingText[0] = '\0';
            inputContext->cursor = 0;
        }
        inputContext->anchor = inputContext->cursor;
    }

    FcitxInputContextManager* manager = inputContext->manager;
    if (manager->callback) {
        FcitxInputContextDeleteSurroundingEvent event;
        event.type = ET_InputContextDeleteSurroundingText;
        event.id = inputContext->id;
        event.inputContext = inputContext;
        event.offset = offset;
        event.length = length;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

FCITX_EXPORT_API
FcitxRect fcitx_input_context_get_cursor_rect(FcitxInputContext* inputContext)
{
    return inputContext->rect;
}

void _fcitx_input_context_focus_in(FcitxInputContext* ic)
{
    ic->focused = true;
    FcitxInputContextManager* manager = ic->manager;
    if (manager->callback) {
        FcitxInputContextEvent event;
        event.type = ET_InputContextFocusIn;
        event.id = ic->id;
        event.inputContext = ic;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

void _fcitx_input_context_focus_out(FcitxInputContext* ic)
{
    ic->focused = false;
    FcitxInputContextManager* manager = ic->manager;
    if (manager->callback) {
        FcitxInputContextEvent event;
        event.type = ET_InputContextFocusOut;
        event.id = ic->id;
        event.inputContext = ic;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

void _fcitx_input_context_destroy(FcitxInputContext* ic)
{
    FcitxInputContextManager* manager = ic->manager;
    if (manager->callback) {
        FcitxInputContextEvent event;
        event.type = ET_InputContextDestroyed;
        event.id = ic->id;
        event.inputContext = ic;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }

    free(ic->surroundingText);
}
