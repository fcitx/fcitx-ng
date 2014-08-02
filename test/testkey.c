#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fcitx-utils/utils.h"
#include "fcitx-utils/keynametable.h"
#include "fcitx-utils/keynametable-compat.h"

#define CHECK_ARRAY_ORDER(ARRAY, COMPARE_FUNC) \
    for (size_t i = 0; i < FCITX_ARRAY_SIZE(ARRAY) - 1; i ++) { \
        assert (COMPARE_FUNC(ARRAY[i], ARRAY[i + 1])); \
    }

int main()
{
#define _STRING_LESS(A, B) (strcmp((A), (B)) < 0)
#define _STRING_LESS_2(A, B) (strcmp((A).name, (B).name) < 0)
#define _SYM_LESS(A, B) ((A).sym < (B).sym)

    CHECK_ARRAY_ORDER(keyNameList, _STRING_LESS);
    CHECK_ARRAY_ORDER(keyNameOffsetByValue, _SYM_LESS);
    CHECK_ARRAY_ORDER(keyNameListCompat, _STRING_LESS_2);

    // Test convert
    for (size_t i = 0; i < FCITX_ARRAY_SIZE(keyValueByNameOffset); i++) {
        assert(fcitx_keysym_to_string(keyValueByNameOffset[i]));
        assert(fcitx_keysym_from_string(keyNameList[i]) == keyValueByNameOffset[i]);
    }

    assert(fcitx_keysym_from_unicode(' ') == FcitxKey_space);
    assert(fcitx_key_is_digit(fcitx_key_parse("1")));
    assert(!fcitx_key_is_digit(fcitx_key_parse("Ctrl+1")));
    assert(!fcitx_key_is_digit(fcitx_key_parse("a")));
    assert(fcitx_key_is_laz(fcitx_key_parse("a")));
    assert(!fcitx_key_is_laz(fcitx_key_parse("Shift_L")));
    assert(fcitx_key_is_uaz(fcitx_key_parse("A")));
    assert(!fcitx_key_is_uaz(fcitx_key_parse("BackSpace")));
    assert(fcitx_key_is_simple(fcitx_key_parse("space")));
    assert(!fcitx_key_is_simple(fcitx_key_parse("EuroSign")));
    assert(fcitx_key_is_modifier(fcitx_key_parse("Control+Alt_L")));
    assert(!fcitx_key_is_modifier(fcitx_key_parse("a")));
    assert(fcitx_key_is_cursor_move(fcitx_key_parse("Left")));
    assert(!fcitx_key_is_cursor_move(fcitx_key_parse("Cancel")));
    assert(fcitx_key_check(fcitx_key_normalize(fcitx_key_parse("Shift+S")), fcitx_key_parse("S")));
    assert(fcitx_key_check(fcitx_key_normalize(fcitx_key_parse("Shift+F4")), fcitx_key_parse("Shift+F4")));
    assert(fcitx_key_check(fcitx_key_normalize(fcitx_key_parse("Ctrl+a")), fcitx_key_parse("Ctrl+A")));
    assert(fcitx_key_check(fcitx_key_normalize(fcitx_key_parse("Alt+Shift+exclam")), fcitx_key_parse("Alt+exclam")));
    assert(fcitx_key_parse("").sym == FcitxKey_None);
    assert(fcitx_key_parse("-").sym == FcitxKey_minus);

    // Test complex parse
    FcitxKeyList* keyList = fcitx_key_list_new_from_string("CTRL_A Control+B Control+Alt+c Control+Alt+Shift+d Control+Alt+Shift+Super+E Super+Alt+=");

    FcitxKey hotkey[] = {
        FCITX_KEY(FcitxKey_A, FcitxKeyState_Ctrl),
        FCITX_KEY(FcitxKey_B, FcitxKeyState_Ctrl),
        FCITX_KEY(FcitxKey_c, FcitxKeyState_Ctrl_Alt),
        FCITX_KEY(FcitxKey_d, FcitxKeyState_Ctrl_Alt_Shift),
        FCITX_KEY(FcitxKey_E, FcitxKeyState_Ctrl_Alt_Shift | FcitxKeyState_Super),
        FCITX_KEY(FcitxKey_equal, FcitxKeyState_Super | FcitxKeyState_Alt),
    };

    for(size_t i = 0; i < FCITX_ARRAY_SIZE(hotkey); i++) {
        assert (fcitx_key_list_check(keyList, hotkey[i]));
    }

    fcitx_key_list_append(keyList, FCITX_KEY(FcitxKey_A, 0));

    char* keyString;
    keyString = fcitx_key_list_to_string(keyList);
    assert(strcmp(keyString, "Control+A Control+B Control+Alt+c Control+Alt+Shift+d Control+Alt+Shift+Super+E Alt+Super+equal A") == 0);
    free(keyString);

    fcitx_key_list_clear(keyList);
    keyString = fcitx_key_list_to_string(keyList);
    assert(strcmp(keyString, "") == 0);
    free(keyString);

    fcitx_key_list_free(keyList);

    return 0;
}
