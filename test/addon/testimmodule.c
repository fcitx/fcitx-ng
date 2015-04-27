#include "fcitx/addon.h"
#include "fcitx/ime.h"
#include "testim-internal.h"
#include "testim_conf.h"
#include "fcitx-config/iniparser.h"

static void* fcitx_test_im_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_test_im_destroy(void* data);
static FcitxConfiguration* fcitx_test_im_list_im(void* data);

FcitxAddonAPIInputMethod testim_inputmethod = {
    .common = {
        .init = fcitx_test_im_init,
        .destroy = fcitx_test_im_destroy,
        .registerCallback = testim_register_functions
    },
    .listInputMethod = fcitx_test_im_list_im
};

void* fcitx_test_im_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(manager);
    FCITX_UNUSED(config);
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

FcitxConfiguration* fcitx_test_im_list_im(void* data)
{
    FCITX_UNUSED(data);
    return fcitx_ini_parse_string((char*) testim_conf, testim_conf_len, NULL);
}
