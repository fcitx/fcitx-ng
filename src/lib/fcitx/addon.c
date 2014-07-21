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

FcitxAddonResolver staticLibraryResolver = {
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
    fcitx_dict_insert_by_str(manager->resolvers, name, resolver, false);
}

void FcitxAddonManagerRegisterDefaultResolver(FcitxAddonManager* mananger)
{
    fcitx_dict_insert_by_str(mananger->resolvers, "staticlibrary", &staticLibraryResolver, false);
    fcitx_dict_insert_by_str(mananger->resolvers, "sharedlibrary", &sharedLibraryResolver, false);
}

void FcitxAddonManagerLoad(FcitxAddonManager* manager)
{
    FcitxStringHashSet* addonNames = FcitxXDGGetFiles("addon", NULL, ".conf");
    HASH_FOREACH(str, sset, FcitxStringHashSet) {
    }
}

void FcitxSharedLibraryResolverList(void* data)
{
}
