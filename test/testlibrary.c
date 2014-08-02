#include <assert.h>
#include <string.h>
#include "fcitx-utils/utils.h"

#define DATA "AAAAAAAAA"
#define MAGIC "MAGIC_TEST_DATA"

FCITX_EXPORT_API
char magic_test[] = MAGIC DATA;

extern int func();

FCITX_EXPORT_API
int func()
{
    return 0;
}

void parser(const char* data, void* arg)
{
    FCITX_UNUSED(arg);
    assert(strcmp(data, DATA) == 0);
}

int main()
{
    FcitxLibrary* lib = fcitx_library_new(0);

    assert(fcitx_library_load(lib, 0));
    assert(fcitx_library_resolve(lib, "func") == func);
    assert(fcitx_library_find_data(lib, "magic_test", MAGIC, strlen(MAGIC), parser, NULL));

    assert(fcitx_library_unload(lib));

    fcitx_library_free(lib);

    return 0;
}
