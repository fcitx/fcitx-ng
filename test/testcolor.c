#include <assert.h>
#include "fcitx-utils/utils.h"

int main()
{
    char colorbuf[FCITX_COLOR_STRING_LENGTH];
    FcitxColor c;
    assert(fcitx_color_parse(&c, "100 101 102"));
    fcitx_color_to_string(&c, colorbuf);
    assert(strcmp(colorbuf, "#646566ff") == 0);
    assert(fcitx_color_parse(&c, "#aabbccdd"));
    fcitx_color_to_string(&c, colorbuf);
    assert(strcmp(colorbuf, "#aabbccdd") == 0);
    return 0;
}
