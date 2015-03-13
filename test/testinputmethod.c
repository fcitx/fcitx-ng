#include <assert.h>
#include "fcitx/ime.h"

#define TEST_FOCUS(ARGS...) \
    do { \
        bool focus_result[] = { ARGS }; \
        for (size_t i = 0; i < FCITX_ARRAY_SIZE(focus_result); i++) { \
            assert(fcitx_input_context_is_focused(ic[i]) == focus_result[i]); \
        } \
    } while(0)

typedef struct _TestIM {
} TestIM;

TestIM testIM;

bool test_im_handle_event(void* _self, const FcitxIMEvent* event)
{
}

int main()
{
    FcitxInputMethodManager* manager = fcitx_input_method_manager_new();

    assert(fcitx_input_method_manager_register(manager, &testIM, "test", "test", "test", test_im_handle_event, 0, "en"));

    fcitx_input_method_manager_unref(manager);

    return 0;
}
