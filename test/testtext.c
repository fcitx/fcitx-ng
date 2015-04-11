#include <assert.h>
#include "fcitx/ui.h"

int main()
{
    FcitxText* text = fcitx_text_new();

    fcitx_text_append(text, 0, "A");
    assert(fcitx_text_size(text) == 1);
    fcitx_text_append_sprintf(text, FTFF_HighLight, "%s", "B");
    assert(fcitx_text_size(text) == 2);
    fcitx_text_clear(text);

    fcitx_text_append(text, 0, "A");
    assert(fcitx_text_size(text) == 1);
    fcitx_text_append_sprintf(text, FTFF_HighLight, "%s", "B");
    assert(fcitx_text_size(text) == 2);
    char* str = fcitx_text_to_string(text);
    assert(strcmp(str, "AB") == 0);
    free(str);

    fcitx_text_unref(text);

    return 0;
}
