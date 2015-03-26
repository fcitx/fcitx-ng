#ifndef __FCITX_INPUTCONTEXT_H__
#define __FCITX_INPUTCONTEXT_H__

#include <fcitx-utils/utils.h>
#include "event.h"
#include <sys/types.h>

typedef struct _FcitxInputContext FcitxInputContext;

typedef enum _FcitxInputContextFocusGroupType
{
    FICFG_Global,
    FICFG_Local,
    FICFG_Independent
} FcitxInputContextFocusGroupType;

typedef struct _FcitxInputContextFocusGroup FcitxInputContextFocusGroup;
typedef struct _FcitxInputContextSharedStatePolicy FcitxInputContextSharedStatePolicy;


typedef void* (*FcitxSetPropertyCallback)(void* oldValue, void* newValue, void* userData);
typedef char* (*FcitxPolicyPropertyKey)(void* value, size_t* len, void* userData);

/** fcitx input context capability flags */
typedef enum _FcitxCapabilityFlag {
    CAPABILITY_NONE = 0,
    CAPABILITY_CLIENT_SIDE_UI = (1 << 0),
    CAPABILITY_PREEDIT = (1 << 1),
    CAPABILITY_CLIENT_SIDE_CONTROL_STATE =  (1 << 2),
    CAPABILITY_PASSWORD = (1 << 3),
    CAPABILITY_FORMATTED_PREEDIT = (1 << 4),
    CAPABILITY_CLIENT_UNFOCUS_COMMIT = (1 << 5),
    CAPABILITY_SURROUNDING_TEXT = (1 << 6),
    CAPABILITY_EMAIL = (1 << 7),
    CAPABILITY_DIGIT = (1 << 8),
    CAPABILITY_UPPERCASE = (1 << 9),
    CAPABILITY_LOWERCASE = (1 << 10),
    CAPABILITY_NOAUTOUPPERCASE = (1 << 11),
    CAPABILITY_URL = (1 << 12),
    CAPABILITY_DIALABLE = (1 << 13),
    CAPABILITY_NUMBER = (1 << 14),
    CAPABILITY_NO_ON_SCREEN_KEYBOARD = (1 << 15),
    CAPABILITY_SPELLCHECK = (1 << 16),
    CAPABILITY_NO_SPELLCHECK = (1 << 17),
    CAPABILITY_WORD_COMPLETION = (1 << 18),
    CAPABILITY_UPPERCASE_WORDS = (1 << 19),
    CAPABILITY_UPPERCASE_SENTENCES = (1 << 20),
    CAPABILITY_ALPHA = (1 << 21),
    CAPABILITY_NAME = (1 << 22)
} FcitxCapabilityFlag;

typedef struct _FcitxKeyEvent
{
    FcitxKey key;
    FcitxKey rawKey;
    int keyCode;
    bool isRelease;
    uint64_t time;
} FcitxKeyEvent;

typedef struct _FcitxInputContextEvent
{
    FcitxEventType type;
    uint32_t id;
    FcitxInputContext* inputContext;
} FcitxInputContextEvent;

typedef struct _FcitxInputContextDeleteSurroundingEvent
{
    FcitxEventType type;
    uint32_t id;
    FcitxInputContext* inputContext;
    int offset;
    unsigned int length;
} FcitxInputContextDeleteSurroundingEvent;

typedef struct _FcitxInputContextCommitStringEvent
{
    FcitxEventType type;
    uint32_t id;
    FcitxInputContext* inputContext;
    const char* commitString;
} FcitxInputContextCommitStringEvent;

typedef struct _FcitxInputContextKeyEvent
{
    FcitxEventType type;
    uint32_t id;
    FcitxInputContext* inputContext;
    FcitxKeyEvent detail;
} FcitxInputContextKeyEvent;

typedef struct _FcitxInputContextManager FcitxInputContextManager;

FcitxInputContextManager* fcitx_input_context_manager_new();
FcitxInputContextManager* fcitx_input_context_manager_ref(FcitxInputContextManager* manager);
void fcitx_input_context_manager_unref(FcitxInputContextManager* manager);

void fcitx_input_context_manager_set_event_dispatcher(FcitxInputContextManager* manager, FcitxDispatchEventCallback callback, FcitxDestroyNotify destroyNotify, void* userData);

FcitxInputContextFocusGroup* fcitx_input_context_focus_group_new(FcitxInputContextManager* manager);
void fcitx_input_context_focus_group_free(FcitxInputContextFocusGroup* group);

FcitxInputContextSharedStatePolicy* fcitx_input_context_shared_state_policy_new(FcitxInputContextManager* manager, int32_t propertyId, FcitxPolicyPropertyKey propertyKey, FcitxDestroyNotify destroyNotify, void* userData);

void fcitx_input_context_manager_set_shared_state_policy(FcitxInputContextManager* manager, FcitxInputContextSharedStatePolicy* policy);

#define FCITX_INVALID_IC_PROPERTY_ID -1

int32_t fcitx_input_context_manager_register_property(FcitxInputContextManager* manager, const char* name, FcitxSetPropertyCallback setProperty, FcitxSetPropertyCallback copyProperty, FcitxClosureFunc propertyDestroyNotify, FcitxDestroyNotify destroyNotify, void* userData);

int32_t fcitx_input_context_manager_lookup_property(FcitxInputContextManager* manager, const char* name);

FcitxInputContext* fcitx_input_context_new(FcitxInputContextManager* manager, uint32_t frontend);

#define FCITX_INVALID_IC 0

/**
 * get ic
 *
 * @param manager
 * @param id 0 for current ic
 * @return FcitxInputContext*
 */
FcitxInputContext* fcitx_input_context_manager_get_input_context(FcitxInputContextManager* manager, uint32_t id);

void fcitx_input_context_focus_in(FcitxInputContext* inputContext);
void fcitx_input_context_focus_out(FcitxInputContext* inputContext);
void fcitx_input_context_set_focus_group(FcitxInputContext* ic, FcitxInputContextFocusGroupType type, FcitxInputContextFocusGroup* group);
void fcitx_input_context_reset(FcitxInputContext* inputContext);
void fcitx_input_context_destroy(FcitxInputContext* inputContext);
uint32_t fcitx_input_context_get_id(FcitxInputContext* inputContext);
void fcitx_input_context_get_uuid(FcitxInputContext* inputContext, uint8_t* uuid);
uint32_t fcitx_input_context_get_capability_flags(FcitxInputContext* inputContext);
void fcitx_input_context_set_capability_flags(FcitxInputContext* inputContext, uint32_t flags);
void fcitx_input_context_set_cursor_rect(FcitxInputContext* inputContext, FcitxRect rect);
bool fcitx_input_context_process_key_event(FcitxInputContext* inputContext, FcitxKeyEvent* key);
void fcitx_input_context_commit_string(FcitxInputContext* inputContext, const char* commitString);
void fcitx_input_context_delete_surrounding_text(FcitxInputContext* inputContext, int offset, unsigned int size);

bool fcitx_input_context_get_surrounding_text(FcitxInputContext* inputContext, const char** str, unsigned int* cursor, unsigned int* anchor);
void fcitx_input_context_set_surrounding_text(FcitxInputContext* inputContext, const char* str, unsigned int cursor, unsigned int anchor);

FcitxRect fcitx_input_context_get_cursor_rect(FcitxInputContext* inputContext);
bool fcitx_input_context_is_focused(FcitxInputContext* inputContext);

void fcitx_input_context_set_property(FcitxInputContext* inputContext, int32_t id, void* data);
void* fcitx_input_context_get_property(FcitxInputContext* inputContext, int32_t id);

#endif // __FCITX_INPUTCONTEXT_H__
