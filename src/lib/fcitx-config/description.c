#include "description.h"

bool is_builtin_type(const char* str)
{
    return  (strcmp(str, "Integer") == 0 ||
             strcmp(str, "String") == 0 || strcmp(str, "File") == 0 || strcmp(str, "Font") == 0 ||
             strcmp(str, "Boolean") == 0 ||
             strcmp(str, "Char") == 0 ||
             strcmp(str, "Enum") == 0 ||
             strcmp(str, "I18NString") == 0 ||
             strcmp(str, "Hotkey") == 0 ||
             strcmp(str, "Color") == 0 ||
             strcmp(str, "List") == 0);
}

void find_structs_callback(FcitxConfiguration* config,
                          const char* path,
                          void* userData)
{
    if (strcmp(path, "DescriptionFile") == 0) {
        return;
    }
    FcitxDescription* desc = userData;
    fcitx_string_hashset_insert(desc->structs, path);
}

void find_structs(FcitxConfiguration* config, FcitxDescription* desc)
{
    desc->structs = fcitx_string_hashset_new();
    // search one level
    fcitx_configuration_foreach(config, "", false, NULL, find_structs_callback, desc);
    // char *structsString = fcitx_string_hashset_join(structs, ',');
    // fprintf(fout, "%s\n", structsString);
}

void struct_attribute_foreach(FcitxConfiguration* config, const char* path, void* userData)
{
    FcitxDescription* desc = userData;
    if (desc->error) {
        return;
    }

    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (!type) {
        asprintf(&desc->errorMessage, "%s misses Type.", path);
        desc->error = true;
        return;
    }

    if (!is_builtin_type(type)) {
        asprintf(&desc->errorMessage, "Invalide Type.");
        desc->error = true;
        return;
    }

    if (strcmp(type, "List") == 0) {
        const char* subType = fcitx_configuration_get_value_by_path(config, "SubType");
        if (!subType) {
            asprintf(&desc->errorMessage, "%s misses SubType.", path);
            desc->error = true;
            return;
        }

        if (strcmp(subType, "List") == 0) {
            asprintf(&desc->errorMessage, "Recursive List is not allowed.");
            desc->error = true;
            return;
        }

        if (!is_builtin_type(subType) && !fcitx_string_hashset_contains(desc->structs, subType)) {
            asprintf(&desc->errorMessage, "Invalide SubType.");
            desc->error = true;
            return;
        }

        fcitx_string_hashset_remove(desc->topLevelStructs, subType);
    }
}

bool structs_foreach(const char* key, size_t keyLen, void** data, void* userData)
{
    FcitxDescription* desc = userData;
    fcitx_configuration_foreach(desc->rootConfig, key, false, "", struct_attribute_foreach, userData);
    return false;
}

void find_top_level_structs(FcitxConfiguration* config, FcitxDescription* desc)
{
    desc->topLevelStructs = fcitx_string_hashset_clone(desc->structs);

    fcitx_dict_foreach(desc->structs, structs_foreach, desc);
}

FCITX_EXPORT_API
void fcitx_description_free(FcitxDescription* desc)
{
    free(desc->errorMessage);
    fcitx_string_hashset_free(desc->structs);
    fcitx_string_hashset_free(desc->topLevelStructs);
    fcitx_configuration_unref(desc->rootConfig);
    free(desc);
}

FCITX_EXPORT_API
FcitxDescription* fcitx_description_parse(FcitxConfiguration* config)
{
    const char* localeDomain = fcitx_configuration_get_value_by_path(config, "DescriptionFile/LocaleDomain");

    FcitxDescription* desc = fcitx_utils_new(FcitxDescription);
    fcitx_utils_string_swap(&desc->localeDomain, localeDomain);
    desc->rootConfig = fcitx_configuration_ref(config);
    do {
        find_structs(config, desc);
        if (desc->error) {
            break;
        }
        find_top_level_structs(config, desc);
        if (desc->error) {
            break;
        }
    } while(0);

    return desc;
}
