#include <assert.h>
#include "fcitx/inputcontext.h"

#define TEST_FOCUS(ARGS...) \
    do { \
        bool focus_result[] = { ARGS }; \
        for (size_t i = 0; i < FCITX_ARRAY_SIZE(focus_result); i++) { \
            assert(fcitx_input_context_is_focused(ic[i]) == focus_result[i]); \
        } \
    } while(0)

int main()
{
    FcitxInputContextManager* manager = fcitx_input_context_manager_new();
    FcitxInputContext *ic[] = { fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL),
                                fcitx_input_context_manager_create_ic(manager, NULL, NULL) };

    FcitxInputContextFocusGroup* group = fcitx_input_context_manager_create_focus_group(manager);
    FcitxInputContextFocusGroup* group2 = fcitx_input_context_manager_create_focus_group(manager);

    fcitx_input_context_set_focus_group(ic[0], FICFG_Global, NULL);
    fcitx_input_context_set_focus_group(ic[1], FICFG_Global, NULL);
    fcitx_input_context_set_focus_group(ic[2], FICFG_Local, group);
    fcitx_input_context_set_focus_group(ic[3], FICFG_Local, group);
    fcitx_input_context_set_focus_group(ic[4], FICFG_Local, group2);
    fcitx_input_context_set_focus_group(ic[5], FICFG_Local, group2);
    fcitx_input_context_set_focus_group(ic[6], FICFG_Independent, NULL);
    fcitx_input_context_set_focus_group(ic[7], FICFG_Independent, NULL);

    TEST_FOCUS(false, false, false, false, false, false, false, false);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[0]));
    TEST_FOCUS(true, false, false, false, false, false, false, false);
    fcitx_input_context_manager_focus_out(manager, fcitx_input_context_get_id(ic[0]));
    TEST_FOCUS(false, false, false, false, false, false, false, false);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[2]));
    TEST_FOCUS(false, false, true, false, false, false, false, false);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[3]));
    TEST_FOCUS(false, false, false, true, false, false, false, false);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[4]));
    TEST_FOCUS(false, false, false, true, true, false, false, false);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[6]));
    TEST_FOCUS(false, false, false, true, true, false, true, false);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[7]));
    TEST_FOCUS(false, false, false, true, true, false, true, true);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[1]));
    TEST_FOCUS(false, true, false, false, false, false, true, true);
    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic[5]));
    TEST_FOCUS(false, false, false, false, false, true, true, true);

    fcitx_input_context_manager_unref(manager);

    return 0;
}
