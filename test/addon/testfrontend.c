#include "fcitx/addon.h"
#include "fcitx/frontend.h"

static void* fcitx_test_frontend_init(FcitxAddonManager* manager);
static void fcitx_test_frontend_destroy(void* data);

FCITX_DEFINE_ADDON(testfrontend, frontend, FcitxAddonAPIFrontend) = {
    .common = {
        .init = fcitx_test_frontend_init,
        .destroy = fcitx_test_frontend_destroy
    }
};

void* fcitx_test_frontend_init(FcitxAddonManager* manager)
{
    FCITX_UNUSED(manager);
    int* dummy = fcitx_utils_new(int);
    return dummy;
}

void fcitx_test_frontend_destroy(void* data)
{
    free(data);
}
