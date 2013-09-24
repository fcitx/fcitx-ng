#include "fcitx-utils/dict.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void foreach_func(const char* key, void* a, void* b)
{
    if (strcmp(key, "B") == 0) {
        assert(strcmp(a, "C") == 0);
    } else if (strcmp(key, "A") == 0) {
        assert(strcmp(a, "D") == 0);
    }
}

void steal_func(const char* key, void* a, void* b)
{
    free(a);
}

int main()
{
    FcitxDict* dict = fcitx_dict_new(free);

    assert(fcitx_dict_insert(dict, "A", strdup("B"), true));
    assert(fcitx_dict_insert(dict, "B", strdup("C"), true));
    assert(!fcitx_dict_insert(dict, "A", strdup("D"), false));
    assert(fcitx_dict_insert(dict, "A", strdup("D"), true));

    fcitx_dict_foreach(dict, foreach_func, NULL);

    assert(fcitx_dict_lookup(dict, "A", NULL));
    assert(!fcitx_dict_lookup(dict, "C", NULL));
    char* str = NULL, *str2 = NULL;
    assert(fcitx_dict_lookup(dict, "A", (void**) &str));
    assert(fcitx_dict_remove(dict, "A", (void**) &str2));
    assert(str == str2);

    assert(str && strcmp(str, "D") == 0);
    free(str);

    assert(fcitx_dict_insert(dict, "C", strdup("E"), true));
    assert(fcitx_dict_size(dict) == 2);
    assert(!fcitx_dict_remove(dict, "D", NULL));
    assert(fcitx_dict_remove(dict, "C", NULL));
    assert(fcitx_dict_size(dict) == 1);

    fcitx_dict_steal_all(dict, steal_func, NULL);

    assert(fcitx_dict_size(dict) == 0);

    fcitx_dict_free(dict);

    return 0;
}
