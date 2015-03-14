#include <assert.h>
#include "fcitx/ime.h"

typedef struct _TestIM {
} TestIM;

TestIM testIM;

bool test_im_handle_event(void* _self, FcitxEvent* event)
{
    FCITX_UNUSED(_self);
    FCITX_UNUSED(event);
    return false;
}

int main()
{
    FcitxInputMethodManager* manager = fcitx_input_method_manager_new();

    fcitx_input_method_manager_create_group(manager, "layout", "jp", "variant", "kana", NULL);

    assert(fcitx_input_method_manager_register(manager, &testIM, "test", "test", "test", test_im_handle_event, 0, "en"));

    const char* ims[] = {
        "name:test",
        "name:test2,layout:fr",
        NULL
    };
    fcitx_input_method_manager_set_input_method_list(manager, 0, ims);

    fcitx_input_method_manager_unref(manager);

    return 0;
}
