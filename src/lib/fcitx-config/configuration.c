#include "configuration.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include <stdlib.h>

struct _FcitxConfiguration
{
    FcitxDict* subitems;
    char* name;
    char* value;
    char* comment;
    int32_t refcount;
};

static void _fcitx_configuration_foreach(FcitxConfiguration* config, const char* path, bool recursive, FcitxConfigurationForeachCallback callback, void* userData);

FCITX_EXPORT_API
FcitxConfiguration* fcitx_configuration_new(const char* name)
{
    FcitxConfiguration* config = fcitx_utils_new(FcitxConfiguration);
    fcitx_utils_string_swap(&config->name, name);
    return fcitx_configuration_ref(config);
}

void fcitx_configuration_free(FcitxConfiguration* config)
{
    fcitx_dict_free(config->subitems);
    free(config->name);
    free(config->value);
    free(config->comment);
    free(config);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxConfiguration, fcitx_configuration);

FcitxConfiguration* _fcitx_configuration_get(FcitxConfiguration* config, const char* path, bool createPath, FcitxConfiguration** parent, const char** lastPath)
{
    if (parent) {
        *parent = NULL;
        *lastPath = NULL;
    }
    while (path[0]) {
        if (path[0] == '/') {
            path ++;
        }
        char* seg = strchrnul(path, '/');
        size_t len = seg - path;
        if (parent) {
            *parent = config;
            *lastPath = path;
        }
        if (!config->subitems || !fcitx_dict_lookup(config->subitems, path, len, &config)) {
            if (!createPath) {
                return NULL;
            } else {
                FcitxConfiguration* newConfig = fcitx_configuration_new(NULL);
                newConfig->name = strndup(path, len);
                if (!config->subitems) {
                    config->subitems = fcitx_dict_new((FcitxDestroyNotify) fcitx_configuration_unref);
                }

                fcitx_dict_insert(config->subitems, path, len, newConfig, false);
                config = newConfig;
            }
        }

        path = seg;
    }

    return config;
}


FCITX_EXPORT_API
FcitxConfiguration* fcitx_configuration_get(FcitxConfiguration* config, const char* path, bool createPath)
{
    return _fcitx_configuration_get(config, path, createPath, NULL, NULL);
}

FCITX_EXPORT_API
void fcitx_configuration_remove(FcitxConfiguration* config, const char* path)
{
    FcitxConfiguration* parent;
    const char* lastPath = NULL;
    config = _fcitx_configuration_get(config, path, false, &parent, &lastPath);
    if (!config) {
        return;
    }

    fcitx_dict_remove_by_str(parent->subitems, lastPath, NULL);
}

const char* fcitx_configuration_get_comment(FcitxConfiguration* config)
{
    return config->comment;
}

FCITX_EXPORT_API
const char* fcitx_configuration_get_name(const FcitxConfiguration* config)
{
    return config->name;
}

FCITX_EXPORT_API
void fcitx_configuration_set_comment(FcitxConfiguration* config, const char* comment)
{
    fcitx_utils_string_swap(&config->comment, comment);
}

FCITX_EXPORT_API
void fcitx_configuration_set_value(FcitxConfiguration* config, const char* value)
{
    fcitx_utils_string_swap(&config->value, value);
}

FCITX_EXPORT_API
const char* fcitx_configuration_get_value(const FcitxConfiguration* config)
{
    return config->value;
}

typedef struct
{
    const char* path;
    char* pathBuf;
    size_t pathLen;
    size_t pathBufLen;
    FcitxConfigurationForeachCallback callback;
    void* userData;
    bool recursive;
} fcitx_configuration_foreach_context;

bool _fcitx_configuration_subitems_foreach(const char* key, size_t keyLen, void** data, void* arg)
{
    fcitx_configuration_foreach_context* context = arg;
    size_t newPathLen = context->pathLen + keyLen + (context->pathLen ? 1 : 0);
    if (newPathLen > context->pathBufLen) {
        context->pathBuf = realloc(context->pathBuf, newPathLen + 1);
        if (context->pathBufLen == 0 && context->pathLen != 0) {
            memcpy(context->pathBuf, context->path, context->pathLen);
            context->pathBuf[context->pathLen] = '/';
        }
        context->pathBufLen = newPathLen;
    }
    memcpy(&context->pathBuf[context->pathLen + (context->pathLen ? 1 : 0)], key, keyLen);
    context->pathBuf[newPathLen] = '\0';

    FcitxConfiguration* config = *data;
    context->callback(config, context->pathBuf, context->userData);

    if (context->recursive) {
        _fcitx_configuration_foreach(config, context->pathBuf, context->recursive, context->callback, context->userData);
    }

    return false;
}

void _fcitx_configuration_foreach(FcitxConfiguration* config, const char* path, bool recursive, FcitxConfigurationForeachCallback callback, void* userData)
{
    if (config->subitems) {
        fcitx_configuration_foreach_context context;
        context.path = path;
        context.pathLen = strlen(path);
        context.pathBuf = NULL;
        context.pathBufLen = 0;
        context.recursive = recursive;
        context.callback = callback;
        context.userData = userData;
        fcitx_dict_foreach(config->subitems, _fcitx_configuration_subitems_foreach, &context);
        free(context.pathBuf);
    }
}

bool fcitx_configuration_has_sub_items(const FcitxConfiguration* config)
{
    return config->subitems && fcitx_dict_size(config->subitems);
}


FCITX_EXPORT_API
void fcitx_configuration_foreach(FcitxConfiguration* config, const char* path, bool recursive, const char* pathPrefix, FcitxConfigurationForeachCallback callback, void* userData)
{
    config = fcitx_configuration_get(config, path, false);
    if (!config) {
        return;
    }

    _fcitx_configuration_foreach(config, pathPrefix ? pathPrefix : "", recursive, callback, userData);
}

typedef struct {
    FcitxConfigurationCompareCallback callback;
    void* userData;
} fcitx_configuration_compare_context;

int _fcitx_configuration_compare(const char* keyA, size_t keyALen, const void* dataA, const char* keyB, size_t keyBLen, const void* dataB, void* userData)
{
    fcitx_configuration_compare_context* context = userData;
    return context->callback(dataA, dataB, context->userData);
}

FCITX_EXPORT_API
void fcitx_configuration_sort(FcitxConfiguration* config, const char* path, FcitxConfigurationCompareCallback compare, void* userData)
{
    config = fcitx_configuration_get(config, path, false);

    if (!config) {
        return;
    }

    if (!compare) {
        fcitx_dict_sort(config->subitems, NULL, NULL);
    } else {
        fcitx_configuration_compare_context context;
        context.callback = compare;
        context.userData = userData;
        fcitx_dict_sort(config->subitems, _fcitx_configuration_compare, &context);
    }
}
