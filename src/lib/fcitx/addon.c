#include "addon.h"
#include "addon-internal.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/stringhashset.h"
#include "fcitx-utils/dict.h"

FcitxAddonResolver sharedLibraryResolver = {
    NULL,
    NULL,
    NULL,
    NULL
};

void FcitxAddonResolverFree(void* data)
{
    FcitxAddonResolver* resolver = data;
    if (resolver->destroyNotify) {
        resolver->destroyNotify(resolver);
    }
}

FcitxAddonManager* FcitxAddonManagerNew()
{
    FcitxAddonManager* mananger = fcitx_utils_new(FcitxAddonManager);
    mananger->resolvers = fcitx_dict_new(FcitxAddonResolverFree);
    return mananger;
}

void FcitxAddonManagerFree(FcitxAddonManager* manager)
{
    fcitx_dict_free(manager->resolvers);
    free(manager);
}

void FcitxAddonManagerRegisterResolver(FcitxAddonManager* manager, const char* name, FcitxAddonResolver* resolver)
{
    fcitx_dict_insert(manager->resolvers, name, resolver, false);
}

void FcitxAddonManagerRegisterDefaultResolver(FcitxAddonManager* mananger, void* data)
{

}

void FcitxAddonManagerLoadCallback(void* value, void* data)
{
    FcitxAddonManager* manager = data;
    FcitxAddonResolver* resolver = value;
    resolver->list(resolver->data);
}

void FcitxAddonManagerLoad(FcitxAddonManager* manager)
{
    fcitx_dict_foreach(manager->resolvers, FcitxAddonManagerLoadCallback, manager);
}

void FcitxSharedLibraryResolverList(void* data)
{
    FcitxAddonResolver* resolver = data;
}
