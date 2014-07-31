#include <assert.h>
#include "helper.h"

static
const char* fcitx_configuration_get_value_by_path_with_default(FcitxConfiguration* config, const char* path, const char* defaultValueUsr, const char* defaultValueSys)
{
    assert(defaultValueSys != NULL);
    const char* value = fcitx_configuration_get_value_by_path(config, path);
    if (value == NULL) {
        value = defaultValueUsr ? defaultValueUsr : defaultValueSys;
    }
    return value;
}

FCITX_EXPORT_API
void fcitx_configuration_get_string(FcitxConfiguration* config, const char* path, const char* defaultValue, char** str)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, defaultValue, "");
    fcitx_utils_string_swap(str, value);
}

FCITX_EXPORT_API
void fcitx_configuration_set_string(FcitxConfiguration* config, const char* path, char** str)
{
    if (!*str) {
        return;
    }
    fcitx_configuration_set_value_by_path(config, path, *str);
}

FCITX_EXPORT_API
void fcitx_configuration_get_integer(FcitxConfiguration* config, const char* path, const char* defaultValue, int* integer)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, defaultValue, "0");
    *integer = atoi(value);
}

FCITX_EXPORT_API
void fcitx_configuration_set_integer(FcitxConfiguration* config, const char* path, const int* integer)
{
    char buf[64];
    sprintf(buf, "%d", *integer);
    fcitx_configuration_set_value_by_path(config, path, buf);
}

FCITX_EXPORT_API
void fcitx_configuration_get_boolean(FcitxConfiguration* config, const char* path, const char* defaultValue, bool* b)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, defaultValue, "False");
    *b = (strcmp(value, "True") == 0);
}

FCITX_EXPORT_API
void fcitx_configuration_set_boolean(FcitxConfiguration* config, const char* path, const bool* b)
{
    fcitx_configuration_set_value_by_path(config, path, *b ? "True" : "False");
}

FCITX_EXPORT_API
void fcitx_configuration_get_char(FcitxConfiguration* config, const char* path, const char* defaultValue, char* chr)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, defaultValue, "");
    *chr = value[0];
}

FCITX_EXPORT_API
void fcitx_configuration_set_char(FcitxConfiguration* config, const char* path, const char* chr)
{
    char buf[2] = {*chr, '\0'};
    fcitx_configuration_set_value_by_path(config, path, buf);
}

FCITX_EXPORT_API
void fcitx_configuration_get_color(FcitxConfiguration* config, const char* path, const char* defaultValue, FcitxColor* color)
{
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, defaultValue, "#000000ff");
    if (!fcitx_color_parse(color, value)) {
        if (!fcitx_color_parse(color, defaultValue)) {
            fcitx_color_parse(color, "#000000ff");
        }
    }
}

FCITX_EXPORT_API
void fcitx_configuration_set_color(FcitxConfiguration* config, const char* path, const FcitxColor* color)
{
    char buf[FCITX_COLOR_STRING_LENGTH];
    fcitx_color_to_string(color, buf);
    fcitx_configuration_set_value_by_path(config, path, buf);
}

FCITX_EXPORT_API
void fcitx_configuration_get_key(FcitxConfiguration* config, const char* path, const char* defaultValue, FcitxKeyList** keyList)
{
    if (!(*keyList)) {
        *keyList = fcitx_key_list_new();
    }
    fcitx_key_list_clear(*keyList);

    const char* value = fcitx_configuration_get_value_by_path(config, path);
    if (value == NULL) {
        value = defaultValue ? defaultValue : "";
    }
    fcitx_key_list_parse(*keyList, value);
}

FCITX_EXPORT_API
void fcitx_configuration_set_key(FcitxConfiguration* config, const char* path, FcitxKeyList** keyList)
{
    if (!*keyList) {
        return;
    }
    char* buf = fcitx_key_list_to_string(*keyList);
    fcitx_configuration_set_value_by_path(config, path, buf);
    free(buf);
}

typedef struct {
    FcitxI18NString* str;
    const char* prefix;
    size_t prefixLen;
} get_i18n_string_foreach_context;

void get_i18n_string_foreach(FcitxConfiguration* config, const char* path, void* userData)
{
    const char* value;
    if ((value = fcitx_configuration_get_value(config)) == NULL) {
        return;
    }

    get_i18n_string_foreach_context* context = userData;
    if (!fcitx_utils_string_starts_with(path, context->prefix)) {
        return;
    }

    if (path[context->prefixLen] != '[') {
        return;
    }

    size_t len = strlen(path);

    if (path[len - 1] != ']') {
        return;
    }

    fcitx_dict_insert(context->str,
                      &path[context->prefixLen + 1],
                      len - context->prefixLen - 2,
                      strdup(fcitx_configuration_get_value(config)),
                      true);
}

FCITX_EXPORT_API
void fcitx_configuration_get_i18n_string(FcitxConfiguration* config, const char* path, const char* defaultValue, FcitxI18NString** pStr)
{
    if (!*pStr) {
        *pStr = fcitx_i18n_string_new();
    }
    FcitxI18NString* str = *pStr;
    fcitx_dict_remove_all(str);
    const char* value = fcitx_configuration_get_value_by_path_with_default(config, path, defaultValue, "");
    fcitx_dict_insert_by_str(str, "", strdup(value), false);

    get_i18n_string_foreach_context context;
    context.str = str;
    context.prefix = path;
    context.prefixLen = strlen(path);
    fcitx_configuration_foreach(config, "", false, "", get_i18n_string_foreach, &context);
}

typedef struct {

    FcitxConfiguration* config;
    const char* path;
} set_i18n_string_foreach_context;

bool set_i18n_string_foreach(const char* key, size_t keyLen, void** data, void* arg)
{
    set_i18n_string_foreach_context* context = arg;
    if (keyLen) {
        char* path = NULL;
        asprintf(&path, "%s[%s]", context->path, key);
        fcitx_configuration_set_value_by_path(context->config, path, (char*) *data);
        free(path);
    } else {
        fcitx_configuration_set_value_by_path(context->config, context->path, (char*) *data);
    }
    return false;
}

FCITX_EXPORT_API
void fcitx_configuration_set_i18n_string(FcitxConfiguration* config, const char* path, FcitxI18NString** str)
{
    if (!*str) {
        return;
    }

    set_i18n_string_foreach_context context;
    context.config = config;
    context.path = path;
    fcitx_dict_foreach(*str, set_i18n_string_foreach, &context);
}

FCITX_EXPORT_API
void fcitx_configuration_get_enum(FcitxConfiguration* config, const char* path, const char** enumStrings, size_t enumCount, uint32_t defaultValue, uint32_t* enumValue)
{
    const char* value = fcitx_configuration_get_value_by_path(config, path);
    if (value == NULL) {
        *enumValue = defaultValue;
        return;
    }
    size_t i = 0;
    for (i = 0; i < enumCount; i++) {
        if (strcmp(value, enumStrings[i]) == 0) {
            break;
        }
    }
    *enumValue = (i == enumCount) ? defaultValue : i;
}

FCITX_EXPORT_API
void fcitx_configuration_set_enum(FcitxConfiguration* config, const char* path, const char** enumStrings, size_t enumCount, uint32_t enumValue)
{
    if (enumValue >= enumCount) {
        return;
    }

    fcitx_configuration_set_value_by_path(config, path, enumStrings[enumValue]);
}

void _fcitx_configuration_list_free(void* data, void* userData)
{
    FcitxDestroyNotify freeFunc = userData;
    freeFunc(data);
    free(data);
}

FCITX_EXPORT_API
void fcitx_configuration_get_list(FcitxConfiguration* config,
                                  const char* path,
                                  FcitxPtrArray** list,
                                  size_t size,
                                  FcitxConfigurationGetFunc loadFunc,
                                  FcitxDestroyNotify freeFunc)
{
    if (!*list) {
        *list = fcitx_ptr_array_new_full(NULL, NULL, _fcitx_configuration_list_free, freeFunc);
    }

    fcitx_ptr_array_clear(*list);

    config = fcitx_configuration_get(config, path, false);
    if (!config) {
        return;
    }

    uint32_t i = 0;
    char buf[64];
    while (true) {
        sprintf(buf, "%u", i);
        FcitxConfiguration* subConfig = fcitx_configuration_get(config, buf, false);
        if (!subConfig) {
            break;
        }

        void* data = fcitx_utils_malloc0(size);
        loadFunc(subConfig, "", NULL, data);
        fcitx_ptr_array_append(*list, data);
        i++;
    }

}

FCITX_EXPORT_API
void fcitx_configuration_set_list(FcitxConfiguration* config, const char* path, FcitxPtrArray* list, FcitxConfigurationSetFunc setFunc)
{
    config = fcitx_configuration_get(config, path, true);
    char buf[64];
    for (uint32_t i = 0; i < list->len; i++) {
        sprintf(buf, "%u", i);
        void* data = fcitx_ptr_array_index(list, i, void);
        FcitxConfiguration* subConfig = fcitx_configuration_get(config, buf, true);
        setFunc(subConfig, "", data);
    }
}
