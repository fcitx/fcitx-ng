#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include <xcb-imdkit/imdkit.h>

typedef struct _FcitxXIM
{
    FcitxInstance* instance;
} FcitxXIM;


static void* fcitx_xim_init(FcitxAddonManager* manager);
static void fcitx_xim_destroy(void* data);

FCITX_DEFINE_ADDON(fcitx_xim, module, FcitxAddonAPICommon) = {
    .init = fcitx_xim_init,
    .destroy = fcitx_xim_destroy
};

void* fcitx_xim_init(FcitxAddonManager* manager)
{
    FcitxXIM* xim = fcitx_utils_new(FcitxXIM);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    xim->instance = instance;

    return xim;
}

void fcitx_xim_destroy(void* data)
{
    FcitxXIM* xim = data;
    free(xim);
}
