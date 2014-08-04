#include "fcitx/addon.h"
#include "fcitx/addon-internal.h"

bool _fcitx_addon_dump(const char* key, size_t keyLen, void** data, void* arg)
{
    FCITX_UNUSED(key);
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(arg);
    FcitxAddon* addon = *(FcitxAddon**) data;
    printf("%s\n", addon->config->addon.name);

    return false;
}

int main ()
{
    FcitxStandardPath* standardPath = fcitx_standard_path_new();
    FcitxAddonManager* manager = fcitx_addon_manager_new(standardPath);
    fcitx_addon_manager_register_default_resolver(manager);

    fcitx_addon_manager_load(manager);
    fcitx_dict_foreach(manager->addons, _fcitx_addon_dump, NULL);

    fcitx_addon_manager_unref(manager);
    fcitx_standard_path_unref(standardPath);
    return 0;
}
