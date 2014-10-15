#include <assert.h>
#include "test-config.h"
#include <fcitx-config/iniparser.h>

int main()
{
    FcitxColor color;
    fcitx_color_parse(&color, "#34567890");
    FcitxTestConfig* testConfig = fcitx_test_config_new();
    // will contain default value
    assert(strcmp(fcitx_i18n_string_match(testConfig->testOption.name, "en_US"), "Default") == 0);
    assert(fcitx_key_list_check(testConfig->testOption.hotKey, FCITX_KEY(FcitxKey_A, FcitxKeyState_Ctrl)));
    assert(strcmp(testConfig->testOption.string, "CTRL_A") == 0);
    assert(testConfig->testOption.integer == 1);
    assert(testConfig->testOption.character == 'a');
    assert(testConfig->testOption.boolean == true);
    assert(testConfig->testOption.list->len == 0);
    assert(memcmp(&testConfig->testOption.color, &color, sizeof(color)) == 0);

    FcitxConfiguration* config = fcitx_configuration_new(NULL);
    fcitx_configuration_set_value_by_path(config, "Test Option/Name", "Name");
    fcitx_configuration_set_value_by_path(config, "Test Option/Name[ja]", "JA");
    fcitx_configuration_set_value_by_path(config, "Test Option/Hot Key", "CTRL_B");
    // maxlength = 10
    fcitx_configuration_set_value_by_path(config, "Test Option/String", "StringString");
    // range 1 - 5
    fcitx_configuration_set_value_by_path(config, "Test Option/Integer", "20");
    fcitx_configuration_set_value_by_path(config, "Test Option/Character", "c");
    fcitx_configuration_set_value_by_path(config, "Test Option/Boolean", "False");
    fcitx_configuration_set_value_by_path(config, "Test Option/Color", "100 200 30");
    fcitx_configuration_set_value_by_path(config, "Test Option/List/0/A", "Sub");
    fcitx_configuration_set_value_by_path(config, "Test Option/List/1/A", "Sub1");
    fcitx_configuration_set_value_by_path(config, "Test Option/List.Integer/0", "5");
    fcitx_configuration_set_value_by_path(config, "Test Option/List.Integer/1", "6");
    fcitx_configuration_set_value_by_path(config, "Test Option/List.Integer/2", "7");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListI18NString/0", "5");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListI18NString/0[zh_CN]", "6");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListI18NString/1", "6");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListI18NString/1[de]", "7");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListHotkey/0", "5");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListHotkey/1", "6");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListString/0", "s0");
    fcitx_configuration_set_value_by_path(config, "Test Option/ListString/1", "s1");
    // load something
    fcitx_test_config_load(testConfig, config);

    // try the updated value
    assert(strcmp(fcitx_i18n_string_match(testConfig->testOption.name, "en_US"), "Name") == 0);
    assert(fcitx_key_list_check(testConfig->testOption.hotKey, FCITX_KEY(FcitxKey_B, FcitxKeyState_Ctrl)));
    assert(strcmp(testConfig->testOption.string, "CTRL_A") == 0);
    assert(testConfig->testOption.integer == 1);
    assert(testConfig->testOption.character == 'c');
    assert(testConfig->testOption.boolean == false);
    assert(testConfig->testOption.list->len == 2);
    fcitx_color_parse(&color, "#64c81eff");
    assert(memcmp(&testConfig->testOption.color, &color, sizeof(color)) == 0);
    assert(strcmp(fcitx_ptr_array_index(testConfig->testOption.list, 0, FcitxEleMentGroup)->a, "Sub") == 0);
    assert(strcmp(fcitx_ptr_array_index(testConfig->testOption.list, 1, FcitxEleMentGroup)->a, "Sub1") == 0);
    assert(*fcitx_ptr_array_index(testConfig->testOption.listInteger, 0, int) == 5);
    assert(*fcitx_ptr_array_index(testConfig->testOption.listInteger, 1, int) == 6);
    assert(*fcitx_ptr_array_index(testConfig->testOption.listInteger, 2, int) == 7);

    FcitxConfiguration* config2 = fcitx_configuration_new(NULL);
    fcitx_test_config_store(testConfig, config2);
    fcitx_ini_print(config2, stdout);

    fcitx_test_config_free(testConfig);
    fcitx_configuration_unref(config2);
    fcitx_configuration_unref(config);

    return 0;
}
