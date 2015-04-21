#include <assert.h>
// #include <ffi.h>
#include "fcitx/addon.h"
#include "fcitx/addon-internal.h"
#include "fcitx/ime.h"
#include "addon/testim.h"

extern FcitxAddonAPIInputMethod testim_inputmethod;

bool _fcitx_addon_dump(const char* key, size_t keyLen, void** data, void* arg)
{
    FCITXGCLIENT_UNUSED(key);
    FCITXGCLIENT_UNUSED(keyLen);
    FCITXGCLIENT_UNUSED(arg);
    FcitxAddon* addon = *(FcitxAddon**) data;
    printf("%s\n", addon->config->addon.name);

    return false;
}

int main (int argc, char* argv[])
{
    if (argc < 3) {
        return 1;
    }

    FcitxStaticAddon staticAddon[] = {
        FCITX_STATIC_ADDON("testim", &testim_inputmethod),
        FCITX_STATIC_ADDON_END()
    };

    setenv("XDG_DATA_HOME", argv[1], true);
    setenv("XDG_DATA_DIRS", argv[1], true);
    setenv("FCITX_ADDON_DIRS", argv[2], true);
    FcitxStandardPath* standardPath = fcitx_standard_path_new();
    FcitxAddonManager* manager = fcitx_addon_manager_new(standardPath);
    fcitx_addon_manager_register_default_resolver(manager, staticAddon);

    fcitx_addon_manager_load(manager);
    fcitx_dict_foreach(manager->addons, _fcitx_addon_dump, NULL);

#if 0
    extern int testim_test_invoke(int* self);
    int self = 5;
    int* pself = &self;
    assert(testim_test_invoke(&self) == 50);

    ffi_type *ffi_types[FCITX_ADDON_FUNCTION_MAX_ARG + 1];
    void* ffi_args[FCITX_ADDON_FUNCTION_MAX_ARG + 1];
    ffi_args[0] = &pself;
    ffi_types[0] = &ffi_type_pointer;
    ffi_type* ffi_rettype = &ffi_type_sint;
    ffi_cif cif;
    int retVal;
    assert(FFI_OK == ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, ffi_rettype, ffi_types));
    ffi_call(&cif, FFI_FN(testim_test_invoke), &retVal, ffi_args);
#endif
    // (self = 5) * 10
    assert(testim_invoke_test_invoke(manager) == 50);
    // strlen("ABC") + self=5
    assert(testim_invoke_test_invoke_with_arg(manager, "ABC") == 8);

    fcitx_addon_manager_unref(manager);
    fcitx_standard_path_unref(standardPath);
    return 0;
}
