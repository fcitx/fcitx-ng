#include <fcitx-config/helper.h>
void fcitx_addon_group_load(FcitxConfiguration* config, const char* path, const char* _dummy, FcitxAddonGroup* data);
void fcitx_addon_group_store(FcitxConfiguration* config, const char* path, FcitxAddonGroup* data);
void fcitx_addon_group_free(FcitxAddonGroup* data);
void fcitx_addon_option_group_load(FcitxConfiguration* config, const char* path, const char* _dummy, FcitxAddonOptionGroup* data);
void fcitx_addon_option_group_store(FcitxConfiguration* config, const char* path, FcitxAddonOptionGroup* data);
void fcitx_addon_option_group_free(FcitxAddonOptionGroup* data);
void fcitx_addon_group_load(FcitxConfiguration* config, const char* path, const char* _dummy, FcitxAddonGroup* data)
{
    config = fcitx_configuration_get(config, path, false);
    const char* defaultValue = NULL;
    defaultValue = NULL;
    fcitx_configuration_get_string(config, "Name", defaultValue, &data->name);
    defaultValue = "";
    fcitx_configuration_get_i18n_string(config, "GeneralName", defaultValue, &data->generalName);
    defaultValue = "";
    fcitx_configuration_get_i18n_string(config, "Comment", defaultValue, &data->comment);
    defaultValue = "InputMethod";
    do {
        size_t enumCount = 4;
        const char* enumStrings[] = {
            "InputMethod",
            "Frontend",
            "Module",
            "UI",
        };
        fcitx_configuration_get_enum(config, "Category", enumStrings, enumCount, 0, &data->category);
    } while(0);
    defaultValue = "True";
    fcitx_configuration_get_boolean(config, "Enabled", defaultValue, &data->enabled);
    defaultValue = NULL;
    fcitx_configuration_get_string(config, "Library", defaultValue, &data->library);
    defaultValue = "SharedLibrary";
    do {
        size_t enumCount = 2;
        const char* enumStrings[] = {
            "SharedLibrary",
            "DBus",
        };
        fcitx_configuration_get_enum(config, "Type", enumStrings, enumCount, 0, &data->type);
    } while(0);
    defaultValue = "";
    fcitx_configuration_get_string(config, "Dependency", defaultValue, &data->dependency);
    defaultValue = "50";
    fcitx_configuration_get_integer(config, "Priority", defaultValue, &data->priority);
    defaultValue = NULL;
    fcitx_configuration_get_list(config, "Options", &data->options, sizeof(FcitxAddonOptionGroup), (FcitxConfigurationGetFunc) fcitx_addon_option_group_load, (FcitxDestroyNotify) fcitx_addon_option_group_free);
    defaultValue = "Self";
    do {
        size_t enumCount = 3;
        const char* enumStrings[] = {
            "Self",
            "Exec",
            "ConfigFile",
        };
        fcitx_configuration_get_enum(config, "IMRegisterMethod", enumStrings, enumCount, 0, &data->imRegisterMethod);
    } while(0);
    defaultValue = "";
    fcitx_configuration_get_string(config, "IMRegisterArgument", defaultValue, &data->imRegisterArgument);
    defaultValue = "";
    fcitx_configuration_get_string(config, "UIFallback", defaultValue, &data->uiFallback);
    defaultValue = "False";
    fcitx_configuration_get_boolean(config, "LoadLocal", defaultValue, &data->loadLocal);
    defaultValue = "False";
    fcitx_configuration_get_boolean(config, "Advance", defaultValue, &data->advance);
}
void fcitx_addon_group_store(FcitxConfiguration* config, const char* path, FcitxAddonGroup* data)
{
    config = fcitx_configuration_get(config, path, true);
    fcitx_configuration_set_string(config, "Name", &data->name);
    fcitx_configuration_set_i18n_string(config, "GeneralName", &data->generalName);
    fcitx_configuration_set_i18n_string(config, "Comment", &data->comment);
    do {
        size_t enumCount = 4;
        const char* enumStrings[] = {
            "InputMethod",
            "Frontend",
            "Module",
            "UI",
        };
        fcitx_configuration_set_enum(config, "Category", enumStrings, enumCount, data->category);
    } while(0);
    fcitx_configuration_set_boolean(config, "Enabled", &data->enabled);
    fcitx_configuration_set_string(config, "Library", &data->library);
    do {
        size_t enumCount = 2;
        const char* enumStrings[] = {
            "SharedLibrary",
            "DBus",
        };
        fcitx_configuration_set_enum(config, "Type", enumStrings, enumCount, data->type);
    } while(0);
    fcitx_configuration_set_string(config, "Dependency", &data->dependency);
    fcitx_configuration_set_integer(config, "Priority", &data->priority);
    fcitx_configuration_set_list(config, "Options", data->options, (FcitxConfigurationSetFunc) fcitx_addon_option_group_store);
    do {
        size_t enumCount = 3;
        const char* enumStrings[] = {
            "Self",
            "Exec",
            "ConfigFile",
        };
        fcitx_configuration_set_enum(config, "IMRegisterMethod", enumStrings, enumCount, data->imRegisterMethod);
    } while(0);
    fcitx_configuration_set_string(config, "IMRegisterArgument", &data->imRegisterArgument);
    fcitx_configuration_set_string(config, "UIFallback", &data->uiFallback);
    fcitx_configuration_set_boolean(config, "LoadLocal", &data->loadLocal);
    fcitx_configuration_set_boolean(config, "Advance", &data->advance);
}
void fcitx_addon_group_free(FcitxAddonGroup* data)
{
    free(data->name);
    fcitx_i18n_string_free(data->generalName);
    fcitx_i18n_string_free(data->comment);
    free(data->library);
    free(data->dependency);
    fcitx_ptr_array_free(data->options);
    free(data->imRegisterArgument);
    free(data->uiFallback);
}
void fcitx_addon_option_group_load(FcitxConfiguration* config, const char* path, const char* _dummy, FcitxAddonOptionGroup* data)
{
    config = fcitx_configuration_get(config, path, false);
    const char* defaultValue = NULL;
    defaultValue = "";
    fcitx_configuration_get_string(config, "Name", defaultValue, &data->name);
    defaultValue = "";
    fcitx_configuration_get_string(config, "Value", defaultValue, &data->value);
}
void fcitx_addon_option_group_store(FcitxConfiguration* config, const char* path, FcitxAddonOptionGroup* data)
{
    config = fcitx_configuration_get(config, path, true);
    fcitx_configuration_set_string(config, "Name", &data->name);
    fcitx_configuration_set_string(config, "Value", &data->value);
}
void fcitx_addon_option_group_free(FcitxAddonOptionGroup* data)
{
    free(data->name);
    free(data->value);
}
FcitxAddonConfig* fcitx_addon_config_new()
{
    FcitxAddonConfig* data = fcitx_utils_new(FcitxAddonConfig);
    fcitx_addon_config_load(data, NULL);
    return data;
}
void fcitx_addon_config_load(FcitxAddonConfig* data, FcitxConfiguration* config)
{
    fcitx_addon_group_load(config, "Addon", NULL, &data->addon);
}
void fcitx_addon_config_store(FcitxAddonConfig* data, FcitxConfiguration* config)
{
    fcitx_addon_group_store(config, "Addon", &data->addon);
}
void fcitx_addon_config_free(FcitxAddonConfig* data)
{
    fcitx_addon_group_free(&data->addon);
    free(data);
}
