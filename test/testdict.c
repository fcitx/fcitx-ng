#include "fcitx-utils/dict.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    FcitxDict* dict = fcitx_dict_new(free);

    fcitx_dict_insert(dict, "A", strdup("B"), true);
    fcitx_dict_insert(dict, "B", strdup("C"), true);
    fcitx_dict_insert(dict, "A", strdup("D"), true);

    assert(fcitx_dict_lookup(dict, "A", NULL));
    assert(!fcitx_dict_lookup(dict, "C", NULL));
    char* str = NULL;
    fcitx_dict_remove(dict, "A", (void**) &str);

    assert(str && strcmp(str, "D") == 0);
    free(str);

    assert(fcitx_dict_size(dict) == 1);

    fcitx_dict_free(dict);

    return 0;
}
