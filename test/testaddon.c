#include "fcitx/addon.h"
#include "fcitx/addon-internal.h"

int main ()
{
    FcitxStandardPath* standardPath = fcitx_standard_path_new();
    FcitxAddonManager* manager = fcitx_addon_manager_new(standardPath);

    fcitx_addon_manager_load(manager);
    for (size_t i = 0; i < manager->addons->len; i ++) {
        printf("%s\n", ((FcitxAddon*) manager->addons->data[i])->config->addon.name);
    }

    fcitx_addon_manager_unref(manager);
    fcitx_standard_path_unref(standardPath);
    return 0;
}
