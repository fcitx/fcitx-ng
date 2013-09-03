#ifndef _FCITX_UTILS_HOTKEY_H_

#include <fcitx-utils/macro.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/types.h>

FCITX_DECL_BEGIN

typedef uint32_t FcitxKeyStates;

typedef struct _FcitxHotkey
{
    FcitxKeySym sym; /** keyval of hotkey */
    FcitxKeyStates state; /** state of hotkey */
    boolean isRelease;
} FcitxHotkey;

typedef struct _FcitxHotkeyList FcitxHotkeyList;

#define FCITX_HOTKEY(SYM, STATE, IS_RELEASE) ((FcitxHotkey) {SYM, STATE, IS_RELEASE})


FcitxHotkey FcitxHotkeyParse(const char* keyString);
char* FcitxHotkeyToString(FcitxHotkey hotkey);
boolean FcitxHotkeyCheck(FcitxHotkey toCheck, FcitxHotkey key);

FcitxHotkeyList* FcitxHotkeyListNew(void);
FcitxHotkeyList* FcitxHotkeyListParse(const char* keyString);
void FcitxHotkeyListFree(FcitxHotkeyList* keyList);
boolean FcitxHotkeyListCheck(FcitxHotkeyList* keyList, FcitxHotkey key);
void FcitxHotkeyListAppend(FcitxHotkeyList* keyList, FcitxHotkey key);
void FcitxHotkeyListClear(FcitxHotkeyList* keyList);
char* FcitxHotkeyListToString(FcitxHotkeyList* keyList);


const char* FcitxKeySymToString (FcitxKeySym keysym);
FcitxKeySym FcitxKeySymFromString(const char* str);

/**
 * convert unicode character to keyval
 *
 * If No matching keysym value found, return Unicode value plus 0x01000000
 * (a convention introduced in the UTF-8 work on xterm).
 *
 * @param wc unicode
 * @return FcitxKeySym
 **/
FcitxKeySym FcitxUnicodeToKeySym (uint32_t wc);

/**
 * convert keyval to unicode character
 *
 * @param keyval keyval
 * @return unicode
 **/
uint32_t FcitxKeySymToUnicode (FcitxKeySym keyval);

FCITX_DECL_END

#endif //  _FCITX_UTILS_HOTKEY_H_
