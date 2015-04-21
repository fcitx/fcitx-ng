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
    FcitxInputContext* ics_by_uuid;
    FcitxInputContextFocusGroup* globalGroup;
    FcitxListHead groups;
    FcitxDispatchEventCallback callback;
    FcitxDestroyNotify destroyNotify;
    void* userData;
    FcitxInputContextSharedStatePolicy* policy;
    size_t policySlot;
    FcitxPtrArray* policies;
    int maxPropertyId;
    FcitxDict* propertyNames;
    FcitxPtrArray* propertySlots;
} FcitxInputContextManager;

typedef struct _FcitxInputContextSharedStatePolicy
{
    FcitxInputContextManager* manager;
    FcitxPolicyPropertyKey propertyKey;
    FcitxDict* propertyDict;
    FcitxDestroyNotify destroyNotify;
    void* userData;
    int32_t id;
} FcitxInputContextSharedStatePolicy;

typedef struct _FcitxInputContextPropertySlot
{
    FcitxSetPropertyCallback setProperty;
    FcitxClosureFunc propertyDestroyNotify;
    FcitxDestroyNotify destroyNotify;
    void* userData;
    FcitxInputContextManager* manager;
    int32_t id;
    FcitxInputContextSharedStatePolicy* policy;
    FcitxSetPropertyCallback copyProperty;
} FcitxInputContextPropertySlot;

typedef struct _FcitxInputContextProperty
{
    void* data;
    FcitxListHead* head;
    FcitxListHead list;
    FcitxInputContext* inputContext;
    FcitxInputContextPropertySlot* slot;
} FcitxInputContextProperty;

typedef struct _FcitxInputContextStates
{
    void* userData;
    void* (*constructor)(void* data);
    void* (*destroyNotify)(void *state, void *data);
    void (*copyToOtherState)(void *state, void *stateOther, void* data);
} FcitxInputContextStates;



static void fcitx_input_context_copy_state(FcitxInputContext* inputContext, FcitxInputContext* sourceInputContext, int32_t propertyId);
static FcitxInputContextFocusGroup* _fcitx_input_context_focus_group_new(FcitxInputContextManager* manager);
static void _fcitx_input_context_set_focus_group(FcitxInputContext* ic, FcitxInputContextFocusGroup* group);
static void _fcitx_input_context_focus_in(FcitxInputContext* ic);
static void _fcitx_input_context_focus_out(FcitxInputContext* ic);
static void _fcitx_input_context_destroy(FcitxInputContext* ic);
static void fcitx_input_context_policy_free(void*);
static void fcitx_input_context_property_free(void*);
static void fcitx_input_context_property_slot_free(void*);

FCITX_EXPORT_API
FcitxInputContextManager* fcitx_input_context_manager_new()
{
    FcitxInputContextManager* manager = fcitx_utils_new(FcitxInputContextManager);
    manager->globalGroup = _fcitx_input_context_focus_group_new(manager);
    fcitx_list_init(&manager->groups);
    manager->policies = fcitx_ptr_array_new(fcitx_input_context_policy_free);
    manager->propertySlots = fcitx_ptr_array_new(fcitx_input_context_property_slot_free);
    manager->propertyNames = fcitx_dict_new(NULL);

    fcitx_input_context_manager_register_property(manager, FCITX_FRONTEND_DATA_PROPERTY, NULL, NULL, NULL, NULL, NULL);

    return fcitx_input_context_manager_ref(manager);
}

void fcitx_input_context_manager_free(FcitxInputContextManager* manager)
{
    while (manager->ics) {
        fcitx_input_context_destroy(manager->ics);
    }

    fcitx_list_entry_foreach_safe(group, FcitxInputContextFocusGroup, &manager->groups, list) {
        fcitx_input_context_focus_group_free(group);
    }

    fcitx_input_context_focus_group_free(manager->globalGroup);

    while (manager->freeList) {
        FcitxInputContext* ic = manager->freeList;
        // mind need to free more?
        manager->freeList = manager->freeList->hh.next;
        free(ic);
    }

    fcitx_dict_free(manager->propertyNames);
    fcitx_ptr_array_free(manager->propertySlots);
    fcitx_ptr_array_free(manager->policies);

    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }

    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputContextManager, fcitx_input_context_manager);

FCITX_EXPORT_API
void fcitx_input_context_manager_set_event_dispatcher(FcitxInputContextManager* manager, FcitxDispatchEventCallback callback, FcitxDestroyNotify destroyNotify, void* userData)
{
    if (manager->destroyNotify) {
        manager->destroyNotify(manager->userData);
    }

    manager->callback = callback;
    manager->destroyNotify = destroyNotify;
    manager->userData = userData;
}

FCITX_EXPORT_API
void fcitx_input_context_manager_foreach(FcitxInputContextManager* manager, FcitxForeachInputContextCallback callback, void* userData)
{
    FcitxInputContext* ic = manager->ics;
    while (ic) {
        if (!callback(ic, userData)) {
            break;
        }

        ic = ic->hh.next;
    }
}

FCITX_EXPORT_API
void fcitx_input_context_manager_destroy_all_input_context(FcitxInputContextManager* manager)
{
    while (manager->ics) {
        fcitx_input_context_destroy(manager->ics);
    }
}

FcitxInputContextFocusGroup* _fcitx_input_context_focus_group_new(FcitxInputContextManager* manager)
{
    FcitxInputContextFocusGroup* group = fcitx_utils_new(FcitxInputContextFocusGroup);
    fcitx_list_init(&group->inputContexts);
    group->manager = manager;

    return group;

}
FCITX_EXPORT_API
FcitxInputContextFocusGroup* fcitx_input_context_focus_group_new(FcitxInputContextManager* manager)
{
    FcitxInputContextFocusGroup* group = _fcitx_input_context_focus_group_new(manager);
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

    bool oldFocus = ic->focused;
    if (ic->focused) {
        fcitx_input_context_focus_out(ic);
    }

    if (ic->group) {
        fcitx_list_remove(&ic->list);
    }

    if (group) {
        fcitx_list_append(&ic->list, &group->inputContexts);
    }
    ic->group = group;
    if (oldFocus) {
        fcitx_input_context_focus_in(ic);
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
int32_t fcitx_input_context_manager_register_property(FcitxInputContextManager* manager, const char* name, FcitxSetPropertyCallback setProperty, FcitxSetPropertyCallback copyProperty, FcitxClosureFunc propertyDestroyNotify, FcitxDestroyNotify destroyNotify, void* userData)
{
    FcitxInputContextPropertySlot* slot = fcitx_utils_new(FcitxInputContextPropertySlot);
    slot->manager = manager;
    slot->propertyDestroyNotify = propertyDestroyNotify;
    slot->setProperty = setProperty;
    slot->copyProperty = copyProperty;
    slot->destroyNotify = destroyNotify;
    slot->userData = userData;
    slot->id = fcitx_ptr_array_size(manager->propertySlots);
    fcitx_ptr_array_append(manager->propertySlots, slot);

    fcitx_dict_insert_data_by_str(manager->propertyNames, name, slot->id, false);

    return slot->id;
}

FCITX_EXPORT_API
int32_t fcitx_input_context_manager_lookup_property(FcitxInputContextManager* manager, const char* name)
{
    intptr_t id = -1;
    fcitx_dict_lookup_by_str(manager->propertyNames, name, &id);

    return (int32_t) id;
}


FCITX_EXPORT_API
FcitxInputContext* fcitx_input_context_new(FcitxInputContextManager* manager, uint32_t frontend, FcitxInputContextDestroyNotify destroyNotify, void* data)
{
    FcitxInputContext* ic = NULL;
    if (manager->freeList) {
        ic = manager->freeList;
        manager->freeList = ic->hh.next;
        uint32_t oldId = ic->id;
        memset(ic, 0, sizeof(*ic));
        ic->id = oldId;
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
    ic->frontendData = data;
    ic->destroyNotify = destroyNotify;
    ic->data = fcitx_ptr_array_new(NULL);
    ic->properties = fcitx_ptr_array_new(fcitx_input_context_property_free);
    ic->preedit = fcitx_text_new();

    uuid_generate(ic->uuid);

    HASH_ADD(hh, manager->ics, id, sizeof(uint32_t), ic);
    HASH_ADD(hh2, manager->ics_by_uuid, uuid, sizeof(uuid_t), ic);

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
FcitxInputContext* fcitx_input_context_manager_get_input_context(FcitxInputContextManager* manager, uint32_t id)
{
    FcitxInputContext* ic = NULL;
    HASH_FIND(hh, manager->ics, &id, sizeof(uint32_t), ic);
    return ic;
}

FCITX_EXPORT_API
FcitxInputContext* fcitx_input_context_manager_get_input_context_by_uuid(FcitxInputContextManager* manager, uint8_t uuid[16])
{
    FcitxInputContext* ic = NULL;
    HASH_FIND(hh2, manager->ics_by_uuid, uuid, 16, ic);
    return ic;
}

FCITX_EXPORT_API
void fcitx_input_context_destroy(FcitxInputContext* inputContext)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (inputContext != fcitx_input_context_manager_get_input_context(manager, inputContext->id)) {
        return;
    }

    fcitx_input_context_focus_out(inputContext);
    _fcitx_input_context_set_focus_group(inputContext, NULL);
    _fcitx_input_context_destroy(inputContext);

    HASH_DEL(manager->ics, inputContext);
    HASH_DELETE(hh2, manager->ics_by_uuid, inputContext);

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

FCITX_EXPORT_API
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

    if (ic->destroyNotify) {
        ic->destroyNotify(ic, ic->frontendData);
    }

    fcitx_text_unref(ic->preedit);
    fcitx_ptr_array_free(ic->data);
    fcitx_ptr_array_free(ic->properties);
    free(ic->surroundingText);
}

FCITX_EXPORT_API
FcitxInputContextSharedStatePolicy* fcitx_input_context_shared_state_policy_new(FcitxInputContextManager* manager, int32_t propertyId, FcitxPolicyPropertyKey propertyKey, FcitxDestroyNotify destroyNotify, void* userData)
{
    if (propertyId < 0 || (size_t) propertyId >= fcitx_ptr_array_size(manager->propertySlots)) {
        return NULL;
    }

    FcitxInputContextPropertySlot* slot = fcitx_ptr_array_index(manager->propertySlots, propertyId, FcitxInputContextPropertySlot*);
    if (slot->policy) {
        return NULL;
    }

    FcitxInputContextSharedStatePolicy* policy = fcitx_utils_new(FcitxInputContextSharedStatePolicy);
    policy->manager = manager;
    policy->propertyKey = propertyKey;
    policy->destroyNotify = destroyNotify;
    policy->userData = userData;
    policy->id = propertyId;
    policy->propertyDict = fcitx_dict_new(free);
    fcitx_ptr_array_append(manager->policies, policy);

    slot->policy = policy;

    return policy;
}

void fcitx_input_context_property_slot_free(void* _slot)
{
    FcitxInputContextPropertySlot* slot = _slot;

    if (slot->destroyNotify) {
        slot->destroyNotify(slot->userData);
    }

    free(slot);
}

void fcitx_input_context_policy_free(void* _policy)
{
    FcitxInputContextSharedStatePolicy* policy = _policy;
    fcitx_dict_free(policy->propertyDict);

    if (policy->destroyNotify) {
        policy->destroyNotify(policy->userData);
    }

    free(policy);
}


void fcitx_input_context_property_free(void* _property)
{
    FcitxInputContextProperty* property = _property;
    if (!property) {
        return;
    }

    FcitxInputContextPropertySlot* slot = property->slot;

    if (property->head) {
        fcitx_list_remove(&property->list);

        FcitxInputContextSharedStatePolicy* policy = slot->policy;

        if (fcitx_list_is_empty(property->head)) {
            size_t len;
            char* key = policy->propertyKey(property->data, &len, policy->userData);
            fcitx_dict_remove(policy->propertyDict, key, len, NULL);
        }

    }

    if (slot->propertyDestroyNotify) {
        slot->propertyDestroyNotify(property->data, slot->userData);
    }

    free(property);
}

FCITX_EXPORT_API
void fcitx_input_context_manager_set_shared_state_policy(FcitxInputContextManager* manager, FcitxInputContextSharedStatePolicy* policy)
{
    if (manager != policy->manager) {
        return;
    }

    manager->policy = policy;
}

FcitxInputContextProperty* fcitx_input_context_get_property_entry(FcitxInputContext* inputContext, int32_t propertyId)
{
    FcitxInputContextManager* manager = inputContext->manager;
    while (fcitx_ptr_array_size(inputContext->properties) <= (size_t) propertyId) {
        fcitx_ptr_array_append(inputContext->properties, NULL);
    }

    FcitxInputContextProperty* property = fcitx_ptr_array_index(inputContext->properties, propertyId, FcitxInputContextProperty*);
    if (!property) {
        property = fcitx_utils_new(FcitxInputContextProperty);
        property->inputContext = inputContext;
        property->slot = fcitx_ptr_array_index(manager->propertySlots, propertyId, FcitxInputContextPropertySlot*);
        fcitx_ptr_array_set(inputContext->properties, propertyId, property);
    }

    return property;
}

FCITX_EXPORT_API
void* fcitx_input_context_set_property(FcitxInputContext* inputContext, int32_t propertyId, void* data)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (propertyId < 0 || (size_t) propertyId >= fcitx_ptr_array_size(manager->propertySlots)) {
        return NULL;
    }

    FcitxInputContextProperty* property = fcitx_input_context_get_property_entry(inputContext, propertyId);
    FcitxInputContextPropertySlot* slot = property->slot;
    FcitxInputContextSharedStatePolicy* policy = slot->policy;

    // maintain policy group
    if (policy) {
        if (property->head) {
            fcitx_list_remove(&property->list);

            if (fcitx_list_is_empty(property->head)) {
                size_t len;
                char* key = policy->propertyKey(property->data, &len, policy->userData);
                fcitx_dict_remove(policy->propertyDict, key, len, NULL);
            }
            property->head = NULL;
        }
    }

    if (slot->setProperty) {
        property->data = slot->setProperty(property->data, data, slot->userData);
    } else {
        property->data = data;
    }

    // maintain policy group
    if (policy) {
        size_t len;
        char* key = policy->propertyKey(property->data, &len, policy->userData);
        if (len) {
            FcitxListHead* head = NULL;
            if (!fcitx_dict_lookup(policy->propertyDict, key, len, &head)) {
                head = fcitx_utils_new(FcitxListHead);
                fcitx_list_init(head);
                fcitx_dict_insert(policy->propertyDict, key, len, head, false);
            }
            fcitx_list_append(&property->list, head);
            property->head = head;
        }
    }

    // if manager has a policy
    if (manager->policy) {
        FcitxInputContextProperty* policyProperty = fcitx_ptr_array_index(inputContext->properties, manager->policy->id, FcitxInputContextProperty*);
        if (policyProperty && policyProperty->head) {
            FcitxListHead* head = policyProperty->head;
            fcitx_list_entry_foreach(key, FcitxInputContextProperty, head, list) {
                if (key->inputContext != inputContext) {
                    fcitx_input_context_copy_state(key->inputContext, inputContext, propertyId == manager->policy->id ? -1 : propertyId);
                }
            }
        }
    }

    return property->data;
}

FCITX_EXPORT_API
void* fcitx_input_context_get_property(FcitxInputContext* inputContext, int32_t propertyId)
{
    FcitxInputContextManager* manager = inputContext->manager;
    if (propertyId < 0 || (size_t) propertyId >= fcitx_ptr_array_size(manager->propertySlots)) {
        return NULL;
    }

    FcitxInputContextProperty* property = fcitx_ptr_array_index(inputContext->properties, propertyId, FcitxInputContextProperty*);

    return property ? property->data : NULL;
}

void fcitx_input_context_copy_state(FcitxInputContext* inputContext, FcitxInputContext* sourceInputContext, int32_t propertyId)
{
    if (inputContext->manager != sourceInputContext->manager) {
        return;
    }

    FcitxInputContextManager* manager = inputContext->manager;

    size_t start = propertyId < 0 ? 0 : propertyId;
    size_t end = propertyId < 0 ? fcitx_ptr_array_size(manager->propertySlots) : ((size_t) (propertyId + 1));

    for (size_t i = start; i < end; i++) {
        FcitxInputContextPropertySlot* slot = fcitx_ptr_array_index(manager->propertySlots, i, FcitxInputContextPropertySlot*);
        if (slot->copyProperty) {
            FcitxInputContextProperty* sourceProperty = fcitx_input_context_get_property_entry(sourceInputContext, i);
            FcitxInputContextProperty* property = fcitx_input_context_get_property_entry(inputContext, i);
            FcitxInputContextPropertySlot* slot = property->slot;
            property->data = slot->copyProperty(property->data, sourceProperty->data, slot->userData);
        }
    }
}

FcitxText* fcitx_input_context_get_preedit(FcitxInputContext* inputContext)
{
    return inputContext->preedit;
}

void fcitx_input_context_update_preedit(FcitxInputContext* inputContext)
{
    FcitxInputContextManager* manager = inputContext->manager;

    if (manager->callback) {
        FcitxInputContextEvent event;
        event.type = ET_InputContextUpdatePreedit;
        event.id = inputContext->id;
        event.inputContext = inputContext;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

void fcitx_input_context_forward_key(FcitxInputContext* inputContext, FcitxKeyEvent* key)
{
    FcitxInputContextManager* manager = inputContext->manager;

    if (manager->callback) {
        FcitxInputContextKeyEvent event;
        event.type = ET_InputContextForwardKey;
        event.id = inputContext->id;
        event.inputContext = inputContext;
        event.detail = *key;
        manager->callback(manager->userData, (FcitxEvent*) &event);
    }
}

