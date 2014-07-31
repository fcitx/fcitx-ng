#include <assert.h>
#include "test-config.h"
#include <fcitx-config/iniparser.h>

int main()
{
    FcitxColor color;
    fcitx_color_parse(&color, "#34567890");
    FcitxTestConfig* testConfig = fcitx_test_config_new();
    // will contain default value
    assert(strcmp(fcitx_i18n_string_match(testConfig->test.name, "en_US"), "Default") == 0);
    assert(fcitx_key_list_check(testConfig->test.hotkey, FCITX_KEY(FcitxKey_A, FcitxKeyState_Ctrl)));
    assert(strcmp(testConfig->test.string, "CTRL_A") == 0);
    assert(testConfig->test.integer == 1);
    assert(testConfig->test.character == 'a');
    assert(testConfig->test.boolean == true);
    assert(testConfig->test.list->len == 0);
    assert(memcmp(&testConfig->test.color, &color, sizeof(color)) == 0);

    FcitxConfiguration* config = fcitx_configuration_new(NULL);
    fcitx_configuration_set_value_by_path(config, "Test/Name", "Name");
    fcitx_configuration_set_value_by_path(config, "Test/Hotkey", "CTRL_B");
    fcitx_configuration_set_value_by_path(config, "Test/String", "String");
    fcitx_configuration_set_value_by_path(config, "Test/Integer", "2");
    fcitx_configuration_set_value_by_path(config, "Test/Character", "c");
    fcitx_configuration_set_value_by_path(config, "Test/Boolean", "False");
    fcitx_configuration_set_value_by_path(config, "Test/Color", "100 200 30");
    fcitx_configuration_set_value_by_path(config, "Test/List/0/A", "Sub");
    fcitx_configuration_set_value_by_path(config, "Test/List/1/A", "Sub1");
    // load something
    fcitx_test_config_load(testConfig, config);

    // try the updated value
    assert(strcmp(fcitx_i18n_string_match(testConfig->test.name, "en_US"), "Name") == 0);
    assert(fcitx_key_list_check(testConfig->test.hotkey, FCITX_KEY(FcitxKey_B, FcitxKeyState_Ctrl)));
    assert(strcmp(testConfig->test.string, "String") == 0);
    assert(testConfig->test.integer == 2);
    assert(testConfig->test.character == 'c');
    assert(testConfig->test.boolean == false);
    assert(testConfig->test.list->len == 2);
    fcitx_color_parse(&color, "#64c81eff");
    assert(memcmp(&testConfig->test.color, &color, sizeof(color)) == 0);
    assert(strcmp(fcitx_ptr_array_index(testConfig->test.list, 0, FcitxElementGroup)->a, "Sub") == 0);
    assert(strcmp(fcitx_ptr_array_index(testConfig->test.list, 1, FcitxElementGroup)->a, "Sub1") == 0);

    FcitxConfiguration* config2 = fcitx_configuration_new(NULL);
    fcitx_test_config_store(testConfig, config2);
    fcitx_ini_print(config2, stdout);

    fcitx_test_config_free(testConfig);
    fcitx_configuration_unref(config2);
    fcitx_configuration_unref(config);

    return 0;
}
