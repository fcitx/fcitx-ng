#include "fcitx/addon.h"
#include "fcitx/ime.h"

static void* fcitx_test_im_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_test_im_destroy(void* data);

FcitxAddonAPIInputMethod testim_inputmethod = {
    .common = {
        .init = fcitx_test_im_init,
        .destroy = fcitx_test_im_destroy
    }
};

void* fcitx_test_im_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(manager);
    FCITX_UNUSED(config);
    int* dummy = fcitx_utils_new(int);
    return dummy;
}

void fcitx_test_im_destroy(void* data)
{
    free(data);
}
