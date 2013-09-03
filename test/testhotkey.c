#include <assert.h>
#include <string.h>
#include "fcitx-utils/keynametable.h"
#include "fcitx-utils/keynametable-compat.h"
#include "fcitx-utils/hotkey.h"

#define CHECK_ARRAY_ORDER(ARRAY, COMPARE_FUNC) \
    for (int i = 0; i < FCITX_ARRAY_SIZE(ARRAY) - 1; i ++) { \
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
    for (int i = 0; i < FCITX_ARRAY_SIZE(keyNameOffsetByValue); i++) {
        const char* keyName = FcitxKeySymToString(keyNameOffsetByValue[0].sym);
        assert(keyName);
        FcitxKeySym sym = FcitxKeySymFromString(keyName);
        assert(sym == keyNameOffsetByValue[0].sym);
    }

    assert(FcitxHotkeyParse("").sym == FcitxKey_None);
    assert(FcitxHotkeyParse("-").sym == FcitxKey_minus);

    // Test complex parse
    FcitxHotkeyList* keyList = FcitxHotkeyListParse("CTRL_A Control+B Control+Alt+c Control+Alt+Shift+d Control+Alt+Shift+Super+E Super+Alt+=");

    FcitxHotkey hotkey[] = {
        FCITX_HOTKEY(FcitxKey_A, FcitxKeyState_Ctrl, false),
        FCITX_HOTKEY(FcitxKey_B, FcitxKeyState_Ctrl, false),
        FCITX_HOTKEY(FcitxKey_c, FcitxKeyState_Ctrl_Alt, false),
        FCITX_HOTKEY(FcitxKey_d, FcitxKeyState_Ctrl_Alt_Shift, false),
        FCITX_HOTKEY(FcitxKey_E, FcitxKeyState_Ctrl_Alt_Shift | FcitxKeyState_Super, false),
        FCITX_HOTKEY(FcitxKey_equal, FcitxKeyState_Super | FcitxKeyState_Alt, false),
    };

    for(int i = 0; i < FCITX_ARRAY_SIZE(hotkey); i++) {
        assert (FcitxHotkeyListCheck(keyList, hotkey[i]));
    }

    char* keyString = FcitxHotkeyListToString(keyList);
    assert(strcmp(keyString, "Control+A Control+B Control+Alt+c Control+Alt+Shift+d Control+Alt+Shift+Super+E Alt+Super+equal") == 0);

    FcitxHotkeyListFree(keyList);

    return 0;
}
