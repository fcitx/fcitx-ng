#ifndef __FCITX_INPUTCONTEXT_H__
#define __FCITX_INPUTCONTEXT_H__

#include <fcitx-utils/macro.h>
#include <fcitx-utils/key.h>
#include <sys/types.h>

/**
 * Input Method State
 **/
typedef enum _FcitxInputContextState {
    CONTEXT_STATE_IS_DISABLED = 0,
    CONTEXT_STATE_IS_KEYBOARD,
    CONTEXT_STATE_IS_IM,
} FcitxInputContextState;

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
} FcitxCapacityFlags;

typedef struct _FcitxKeyEvent
{
    FcitxKey key;
    FcitxKey rawKey;
    int keyCode;
    boolean isRelease;
} FcitxKeyEvent;

FcitxInputContext* FcitxInputContextNew();
FcitxInputContextState FcitxInputContextGetState(FcitxInputContext* inputContext);
FcitxCapacityFlags FcitxInputContextGetCapacityFlags(FcitxInputContext* inputContext);
void FcitxInputContextProcessKeyEvent(FcitxInputContext* inputContext, FcitxKeyEvent* event);
void FcitxInputContextCommitString(FcitxInputContext* inputContext, const char* commitString);
void FcitxInputContextDeleteSurroundingText(FcitxInputContext* inputContext, int offset, unsigned int size);
void FcitxInputContextGetSurroundingText(FcitxInputContext* inputContext, char** str, unsigned int* cursor, unsigned int* anchor);
pid_t FcitxInputContextGetPid(FcitxInputContext* inputContext);
void FcitxInputContextReset(FcitxInputContext* inputContext);

#endif // __FCITX_INPUTCONTEXT_H__
