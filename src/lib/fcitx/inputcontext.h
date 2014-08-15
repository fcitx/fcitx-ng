#ifndef __FCITX_INPUTCONTEXT_H__
#define __FCITX_INPUTCONTEXT_H__

#include <fcitx-utils/utils.h>
#include <sys/types.h>

typedef struct _FcitxInputContext FcitxInputContext;


/** fcitx input context capability flags */
typedef enum _FcitxCapabilityFlags {
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
} FcitxCapabilityFlags;

typedef struct _FcitxKeyEvent
{
    FcitxKey key;
    FcitxKey rawKey;
    int keyCode;
    bool isRelease;
} FcitxKeyEvent;

typedef struct _FcitxInputContextManager FcitxInputContextManager;
typedef void (*FcitxInputContextFillDataCallback)(FcitxInputContext* context, void* userData);

FcitxInputContextManager* fcitx_input_context_manager_new();
FcitxInputContextManager* fcitx_input_context_manager_ref(FcitxInputContextManager* manager);
void fcitx_input_context_manager_unref(FcitxInputContextManager* manager);
FcitxInputContext* fcitx_input_context_manager_create_ic(FcitxInputContextManager* manager,
                                                         FcitxInputContextFillDataCallback callback,
                                                         void* data);
void fcitx_input_context_manager_focus_in(FcitxInputContextManager* manager, uint32_t id);
void fcitx_input_context_manager_focus_out(FcitxInputContextManager* manager, uint32_t id);

#define FCITX_INVALID_IC 0

/**
 * get ic
 *
 * @param manager
 * @param id 0 for current ic
 * @return FcitxInputContext*
 */
FcitxInputContext* fcitx_input_context_manager_get_ic(FcitxInputContextManager* manager, uint32_t id);

void fcitx_input_context_destroy(FcitxInputContext* inputContext);
uint32_t fcitx_input_context_get_id(FcitxInputContext* inputContext);
uint32_t fcitx_input_context_get_capability_flags(FcitxInputContext* inputContext);
void fcitx_input_context_process_event(FcitxInputContext* inputContext, FcitxKeyEvent* event);
void fcitx_input_context_commit_string(FcitxInputContext* inputContext, const char* commitString);
void fcitx_input_context_delete_surrounding_text(FcitxInputContext* inputContext, int offset, unsigned int size);
void fcitx_input_context_get_surrounding_text(FcitxInputContext* inputContext, char** str, unsigned int* cursor, unsigned int* anchor);
void fcitx_input_context_update_preedit(FcitxInputContext* inputContext);
FcitxRect fcitx_input_context_get_window_rect(FcitxInputContext* inputContext);

#endif // __FCITX_INPUTCONTEXT_H__
