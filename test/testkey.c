#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fcitx-utils/keynametable.h"
#include "fcitx-utils/keynametable-compat.h"
#include "fcitx-utils/key.h"

#define CHECK_ARRAY_ORDER(ARRAY, COMPARE_FUNC) \
    for (size_t i = 0; i < FCITX_ARRAY_SIZE(ARRAY) - 1; i ++) { \
        assert (COMPARE_FUNC(ARRAY[i], ARRAY[i + 1])); \
    }

int main(int argc, char* argv[])
{
#define _STRING_LESS(A, B) (strcmp((A), (B)) < 0)
#define _STRING_LESS_2(A, B) (strcmp((A).name, (B).name) < 0)
#define _SYM_LESS(A, B) ((A).sym < (B).sym)

    CHECK_ARRAY_ORDER(keyNameList, _STRING_LESS);
    CHECK_ARRAY_ORDER(keyNameOffsetByValue, _SYM_LESS);
    CHECK_ARRAY_ORDER(keyNameListCompat, _STRING_LESS_2);

    // Test convert
    for (size_t i = 0; i < FCITX_ARRAY_SIZE(keyValueByNameOffset); i++) {
        assert(FcitxKeySymToString(keyValueByNameOffset[i]));
        assert(FcitxKeySymFromString(keyNameList[i]) == keyValueByNameOffset[i]);
    }

    assert(FcitxUnicodeToKeySym(' ') == FcitxKey_space);
    assert(FcitxKeyIsDigit(FcitxKeyParse("1")));
    assert(!FcitxKeyIsDigit(FcitxKeyParse("a")));
    assert(FcitxKeyIsLAZ(FcitxKeyParse("a")));
    assert(!FcitxKeyIsLAZ(FcitxKeyParse("Shift_L")));
    assert(FcitxKeyIsUAZ(FcitxKeyParse("A")));
    assert(!FcitxKeyIsUAZ(FcitxKeyParse("BackSpace")));
    assert(FcitxKeyIsSimple(FcitxKeyParse("space")));
    assert(!FcitxKeyIsSimple(FcitxKeyParse("EuroSign")));
    assert(FcitxKeyIsModifierCombine(FcitxKeyParse("Control+Alt_L")));
    assert(!FcitxKeyIsModifierCombine(FcitxKeyParse("a")));
    assert(FcitxKeyIsCursorMove(FcitxKeyParse("Left")));
    assert(!FcitxKeyIsCursorMove(FcitxKeyParse("Cancel")));
    assert(FcitxKeyCheck(FcitxKeyNormalize(FcitxKeyParse("Shift+S")), FcitxKeyParse("S")));
    assert(FcitxKeyParse("").sym == FcitxKey_None);
    assert(FcitxKeyParse("-").sym == FcitxKey_minus);

    // Test complex parse
    FcitxKeyList* keyList = FcitxKeyListParse("CTRL_A Control+B Control+Alt+c Control+Alt+Shift+d Control+Alt+Shift+Super+E Super+Alt+=");

    FcitxKey hotkey[] = {
        FCITX_KEY(FcitxKey_A, FcitxKeyState_Ctrl),
        FCITX_KEY(FcitxKey_B, FcitxKeyState_Ctrl),
        FCITX_KEY(FcitxKey_c, FcitxKeyState_Ctrl_Alt),
        FCITX_KEY(FcitxKey_d, FcitxKeyState_Ctrl_Alt_Shift),
        FCITX_KEY(FcitxKey_E, FcitxKeyState_Ctrl_Alt_Shift | FcitxKeyState_Super),
        FCITX_KEY(FcitxKey_equal, FcitxKeyState_Super | FcitxKeyState_Alt),
    };

    for(size_t i = 0; i < FCITX_ARRAY_SIZE(hotkey); i++) {
        assert (FcitxKeyListCheck(keyList, hotkey[i]));
    }

    char* keyString = FcitxKeyListToString(keyList);
    assert(strcmp(keyString, "Control+A Control+B Control+Alt+c Control+Alt+Shift+d Control+Alt+Shift+Super+E Alt+Super+equal") == 0);
    free(keyString);

    FcitxKeyListFree(keyList);

    return 0;
}
