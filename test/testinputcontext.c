#include <assert.h>
#include "fcitx/inputcontext.h"

#define TEST_FOCUS(ARGS...) \
    do { \
        bool focus_result[] = { ARGS }; \
        for (size_t i = 0; i < FCITX_ARRAY_SIZE(focus_result); i++) { \
            assert(fcitx_input_context_is_focused(ic[i]) == focus_result[i]); \
        } \
    } while(0)


static uint32_t flag;

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

bool dispatch_ic_event(void* data, FcitxEvent* event)
{
    if (event->type == ET_InputContextCapabilityChanged) {
        FcitxInputContextEvent* icEvent = (FcitxInputContextEvent*) event;
        flag = fcitx_input_context_get_capability_flags(icEvent->inputContext);
    }
    return true;
}

int main()
{
    FcitxInputContextManager* manager = fcitx_input_context_manager_new();

    fcitx_input_context_manager_set_event_dispatcher(manager, dispatch_ic_event, NULL, NULL);
    FcitxInputContext *ic[] = { fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0),
                                fcitx_input_context_new(manager, 0) };

    fcitx_input_context_destroy(ic[7]);
    ic[7] = fcitx_input_context_new(manager, 0);

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


    fcitx_input_context_set_property(ic[1], id, "Chrome");
    fcitx_input_context_set_property(ic[0], id, "Chrome");

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

    fcitx_input_context_set_capability_flags(ic[1], CAPABILITY_DIGIT);
    assert(flag == CAPABILITY_DIGIT);
    flag = 0;
    fcitx_input_context_set_capability_flags(ic[1], CAPABILITY_DIGIT);
    assert(flag == 0);

    // surroundingText
    fcitx_input_context_set_capability_flags(ic[0], CAPABILITY_SURROUNDING_TEXT);
    fcitx_input_context_set_surrounding_text(ic[0], "abcd", 1, 1);
    const char* surroundingText;
    unsigned int anchor, cursor;
    assert(fcitx_input_context_get_surrounding_text(ic[0], &surroundingText, &anchor, &cursor));
    assert(strcmp(surroundingText, "abcd") == 0);
    assert(anchor == 1 && cursor == 1);

    fcitx_input_context_delete_surrounding_text(ic[0], -1, 2);
    assert(fcitx_input_context_get_surrounding_text(ic[0], &surroundingText, &anchor, &cursor));
    assert(strcmp(surroundingText, "cd") == 0);
    assert(anchor == 0 && cursor == 0);

    // cursor rect
    FcitxRect rect = {0, 0, 1, 1}, rect2;
    fcitx_input_context_set_cursor_rect(ic[2], rect);
    rect2 = fcitx_input_context_get_cursor_rect(ic[2]);
    assert(rect.x1 == rect2.x1 && rect.x2 == rect2.x2 && rect.y1 == rect2.y1 && rect.y2 == rect2.y2);

    fcitx_input_context_manager_unref(manager);

    return 0;
}
