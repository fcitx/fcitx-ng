#ifndef _FCITX_UTILS_KEY_H_

#include <fcitx-utils/macro.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/types.h>

FCITX_DECL_BEGIN

typedef uint32_t FcitxKeyStates;

typedef struct _FcitxKey
{
    FcitxKeySym sym; /** keyval of hotkey */
    FcitxKeyStates state; /** state of hotkey */
} FcitxKey;

typedef struct _FcitxKeyList FcitxKeyList;

#define FCITX_KEY(SYM, STATE) ((FcitxKey) {SYM, STATE})


FcitxKey FcitxKeyParse(const char* keyString);
char* FcitxKeyToString(FcitxKey hotkey);
boolean FcitxKeyCheck(FcitxKey toCheck, FcitxKey key);

boolean FcitxKeyIsDigit(FcitxKey key);
boolean FcitxKeyIsUAZ(FcitxKey key);
boolean FcitxKeyIsLAZ(FcitxKey key);
boolean FcitxKeyIsSimple(FcitxKey key);
boolean FcitxKeyIsModifierCombine(FcitxKey key);
boolean FcitxKeyIsCursorMove(FcitxKey key);

FcitxKey FcitxKeyNormalize(FcitxKey key);

FcitxKeyList* FcitxKeyListNew(void);
FcitxKeyList* FcitxKeyListParse(const char* keyString);
void FcitxKeyListFree(FcitxKeyList* keyList);
boolean FcitxKeyListCheck(FcitxKeyList* keyList, FcitxKey key);
void FcitxKeyListAppend(FcitxKeyList* keyList, FcitxKey key);
void FcitxKeyListClear(FcitxKeyList* keyList);
char* FcitxKeyListToString(FcitxKeyList* keyList);


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

#endif //  _FCITX_UTILS_KEY_H_
