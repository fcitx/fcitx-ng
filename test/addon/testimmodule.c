#include "fcitx/addon.h"
#include "fcitx/ime.h"
#include "testim-internal.h"

static void* fcitx_test_im_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_test_im_destroy(void* data);

FcitxAddonAPIInputMethod testim_inputmethod = {
    .common = {
        .init = fcitx_test_im_init,
        .destroy = fcitx_test_im_destroy,
        .registerCallback = testim_register_functions
    }
};

void* fcitx_test_im_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITXGCLIENT_UNUSED(manager);
    FCITXGCLIENT_UNUSED(config);
    int* dummy = fcitx_utils_new(int);
    *dummy = 5;
    return dummy;
}

void fcitx_test_im_destroy(void* data)
{
    free(data);
}

int testim_test_invoke(int* self)
{
    return *self * 10;
}

uint32_t testim_test_invoke_with_arg(int* self, char* arg0)
{
    return strlen(arg0) + *self;
}
