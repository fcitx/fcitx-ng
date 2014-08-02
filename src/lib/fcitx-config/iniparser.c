#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "fcitx-utils/utils.h"
#include "configuration.h"

FCITX_EXPORT_API
FcitxConfiguration* fcitx_ini_parse(FILE* fp, FcitxConfiguration* config)
{
    char* lineBuf = NULL;
    size_t lineBufLen = 0;

    char* currentGroup = NULL;
    if (!config) {
        config = fcitx_configuration_new(NULL);
    }

    while (getline(&lineBuf, &lineBufLen, fp) != -1) {
        char* trimmedLine = fcitx_utils_inplace_trim(lineBuf);
        if (trimmedLine[0] == '#') {
            continue;
        }

        size_t len = strlen(trimmedLine);
        char* equalPos = NULL;

        if (trimmedLine[0] == '[' && trimmedLine[len - 1] == ']') {
            trimmedLine[len - 1] = '\0';
            trimmedLine ++;
            fcitx_utils_string_swap(&currentGroup, trimmedLine);
        } else if ((equalPos = strchr(trimmedLine, '='))) {
            const char* name = trimmedLine;
            *equalPos = 0;
            char* value = equalPos + 1;
            char* end = trimmedLine + len;
            char* tofree = NULL;
            // having quote at beginning and end, escape
            if (end - value >= 2 && value[0] == '"' && end[-1] == '"') {
                end[-1] = '\0';
                value++;
                char* replaced = fcitx_utils_string_replace(value, "\\\"", "\"", false);
                if (replaced != NULL) {
                    value = replaced;
                    tofree = replaced;
                }
            }

            char* replaced = fcitx_utils_string_replace(value, "\\\n", "\n", true);
            if (replaced != NULL) {
                value = replaced;
                free(tofree);
                tofree = replaced;
            }

            if (currentGroup) {
                char* path = NULL;
                asprintf(&path, "%s/%s", currentGroup, name);
                fcitx_configuration_set_value_by_path(config, path, value);
                free(path);
            } else {
                fcitx_configuration_set_value_by_path(config, name, value);
            }

            free(tofree);
        }
    }

    free(currentGroup);
    free(lineBuf);

    return config;
}

void _fcitx_ini_has_sub_value_callback(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITX_UNUSED(path);
    bool* hasSubValue = userData;

    const char* value = fcitx_configuration_get_value(config);
    if (value) {
        *hasSubValue = true;
    }
}

void _fcitx_ini_foreach_option_callback(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITX_UNUSED(path);
    FILE* fp = userData;

    const char* value = fcitx_configuration_get_value(config);
    if (value) {
        const char* comment = fcitx_configuration_get_comment(config);
        if (comment && !strchr(comment, '\n')) {
            fprintf(fp, "# %s\n", comment);
        }
        char* tofree = NULL;
        tofree = fcitx_utils_string_replace(value, "\n", "\\\n", true);

        if (tofree) {
            value = tofree;
        }

        char* quoteEsacpe = fcitx_utils_string_replace(value, "\"", "\\\"", true);
        if (quoteEsacpe) {
            free(tofree);
            value = tofree = quoteEsacpe;
        }

        if (quoteEsacpe) {
            fprintf(fp, "%s=\"%s\"\n", fcitx_configuration_get_name(config), value);
        } else {
            fprintf(fp, "%s=%s\n", fcitx_configuration_get_name(config), value);
        }

        free(tofree);
    }
}

void _fcitx_ini_foreach_callback(FcitxConfiguration* config, const char* path, void* userData)
{
    FILE* fp = userData;

    bool hasSubValue = false;
    fcitx_configuration_foreach(config, "", false, NULL, _fcitx_ini_has_sub_value_callback, &hasSubValue);
    if (hasSubValue) {
        if (path[0]) {
            fprintf(fp, "[%s]\n", path);
        }
        fcitx_configuration_foreach(config, "", false, NULL, _fcitx_ini_foreach_option_callback, fp);
    }

    fcitx_configuration_foreach(config, "", false, path, _fcitx_ini_foreach_callback, fp);
}

FCITX_EXPORT_API
void fcitx_ini_print(FcitxConfiguration* config, FILE* fp)
{
    _fcitx_ini_foreach_callback(config, "", fp);
}
