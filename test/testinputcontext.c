#include <assert.h>
#include "fcitx/inputcontext.h"

#define TEST_FOCUS(ARGS...) \
    do { \
        bool focus_result[] = { ARGS }; \
        for (size_t i = 0; i < FCITX_ARRAY_SIZE(focus_result); i++) { \
            assert(fcitx_input_context_is_focused(ic[i]) == focus_result[i]); \
        } \
    } while(0)

void* set_string_property(void* old, void* newValue, void* userData)
{
    FCITX_UNUSED(userData);
    free(old);
    return fcitx_utils_strdup(newValue);
}

char* application_name_property_key(void* data, size_t* len, void* userData)
{
    FCITX_UNUSED(userData);
    *len = strlen(data);
    return data;
}

void string_free(void* data, void* userData)
{
    FCITX_UNUSED(userData);
    free(data);
}

int main()
{
    FcitxInputContextManager* manager = fcitx_input_context_manager_new();
    FcitxInputContext *ic[] = { fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0) };

    FcitxInputContextFocusGroup* group = fcitx_input_context_focus_group_new(manager);
    FcitxInputContextFocusGroup* group2 = fcitx_input_context_focus_group_new(manager);

    int32_t id =
        fcitx_input_context_manager_register_property(manager,
                                                      "appName",
                                                      set_string_property,
                                                      NULL,
                                                      string_free,
                                                      NULL,
                                                      NULL);
    int32_t id2 =
        fcitx_input_context_manager_register_property(manager,
                                                      "testCopy",
                                                      set_string_property,
                                                      set_string_property,
                                                      string_free,
                                                      NULL,
                                                      NULL);

    assert(id == fcitx_input_context_manager_lookup_property(manager, "appName"));

    FcitxInputContextSharedStatePolicy* policy = fcitx_input_context_shared_state_policy_new(manager, id, application_name_property_key, NULL, NULL);

    fcitx_input_context_set_property(ic[0], id, "Firefox");
    fcitx_input_context_set_property(ic[1], id, "Firefox");

    fcitx_input_context_set_property(ic[0], id2, "Test");
    char* testValue = fcitx_input_context_get_property(ic[1], id2);
    assert(testValue == NULL);

    fcitx_input_context_manager_set_shared_state_policy(manager, policy);

    fcitx_input_context_set_property(ic[1], id2, "Test");
    testValue = fcitx_input_context_get_property(ic[0], id2);
    assert(testValue && strcmp(testValue, "Test") == 0);

    fcitx_input_context_set_focus_group(ic[0], FICFG_Global, NULL);
    fcitx_input_context_set_focus_group(ic[1], FICFG_Global, NULL);
    fcitx_input_context_set_focus_group(ic[2], FICFG_Local, group);
    fcitx_input_context_set_focus_group(ic[3], FICFG_Local, group);
    fcitx_input_context_set_focus_group(ic[4], FICFG_Local, group2);
    fcitx_input_context_set_focus_group(ic[5], FICFG_Local, group2);
    fcitx_input_context_set_focus_group(ic[6], FICFG_Independent, NULL);
    fcitx_input_context_set_focus_group(ic[7], FICFG_Independent, NULL);

    TEST_FOCUS(false, false, false, false, false, false, false, false);
    fcitx_input_context_focus_in(ic[0]);
    TEST_FOCUS(true, false, false, false, false, false, false, false);
    fcitx_input_context_focus_out(ic[0]);
    TEST_FOCUS(false, false, false, false, false, false, false, false);
    fcitx_input_context_focus_in(ic[2]);
    TEST_FOCUS(false, false, true, false, false, false, false, false);
    fcitx_input_context_focus_in(ic[3]);
    TEST_FOCUS(false, false, false, true, false, false, false, false);
    fcitx_input_context_focus_in(ic[4]);
    TEST_FOCUS(false, false, false, true, true, false, false, false);
    fcitx_input_context_focus_in(ic[6]);
    TEST_FOCUS(false, false, false, true, true, false, true, false);
    fcitx_input_context_focus_in(ic[7]);
    TEST_FOCUS(false, false, false, true, true, false, true, true);
    fcitx_input_context_focus_in(ic[1]);
    TEST_FOCUS(false, true, false, false, false, false, true, true);
    fcitx_input_context_focus_in(ic[5]);
    TEST_FOCUS(false, false, false, false, false, true, true, true);

    fcitx_input_context_manager_unref(manager);

    return 0;
}
