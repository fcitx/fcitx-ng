#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"

const char configText[] =
"Addon=test2\n"
"[Addon]\n"
"Name=test\n"
"[Addon/X]\n"
"Enum1=test1\n"
"Enum4=test4\n"
"Enum2=test2\n"
"Enum3=test3\n"
"[Addon/Enum]\n"
"Enum1=test1\n"
"Enum2=test2\n"
"Enum3=test3\n"
"Enum4=test4\n";

void dump(FcitxConfiguration* config, const char* path, void* data)
{
    FCITX_UNUSED(data);
    printf("%s=%s\n", path, fcitx_configuration_get_value(config));
}

int main()
{
    FcitxConfiguration* config = fcitx_configuration_new(NULL);
    fcitx_configuration_set_value_by_path(config, "Addon/Name", "test");
    fcitx_configuration_set_value_by_path(config, "Addon/X/Enum1", "test1");
    fcitx_configuration_set_value_by_path(config, "Addon/X/Enum4", "test4");
    fcitx_configuration_set_value_by_path(config, "Addon/X/Enum2", "test2");
    fcitx_configuration_set_value_by_path(config, "Addon/X/Enum3", "test3");
    fcitx_configuration_set_value_by_path(config, "Addon/Enum/Enum1", "test1");
    fcitx_configuration_set_value_by_path(config, "Addon/Enum/Enum4", "test4");
    fcitx_configuration_set_value_by_path(config, "Addon/Enum/Enum2", "test2");
    fcitx_configuration_set_value_by_path(config, "Addon/Enum/Enum3", "test3");
    assert(0 == strcmp("test", fcitx_configuration_get_value_by_path(config, "Addon/Name")));

    fcitx_configuration_remove(config, "Addon/Name");
    assert(NULL == fcitx_configuration_get_value_by_path(config, "Addon/Name"));

    fcitx_configuration_sort(config, "Addon/Enum", NULL, NULL);
    fcitx_configuration_foreach(config, "Addon/Enum", false, NULL, dump, NULL);
    fcitx_configuration_set_value_by_path(config, "Addon/Name", "test");
    fcitx_configuration_set_value_by_path(config, "Addon", "test2");

    fcitx_configuration_unref(config);

    FILE* fp = fmemopen((void*) configText, sizeof(configText), "r");
    config = fcitx_ini_parse(fp, NULL);
    fclose(fp);

    fcitx_ini_print(config, stdout);

    fcitx_configuration_unref(config);

    return 0;
}
