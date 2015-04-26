#include <assert.h>
#include "fcitx/ime.h"

int main()
{
    FcitxInputMethodManager* manager = fcitx_input_method_manager_new(NULL);

    fcitx_input_method_manager_create_group(manager, "layout", "jp", "variant", "kana", NULL);

    const char* ims[] = {
        "name:test",
        "name:test2,layout:fr",
        NULL
    };
    fcitx_input_method_manager_set_input_method_list(manager, 0, ims);

    fcitx_input_method_manager_unref(manager);

    return 0;
}
