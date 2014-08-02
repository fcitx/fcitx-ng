#include <stdio.h>
#include "main.h"
#include "fcitx-utils/utils.h"
#include "fcitx-config/configuration.h"

static void print_pot_header()
{
    fprintf(fout, "# SOME DESCRIPTIVE TITLE.\n"
           "# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER\n"
           "# This file is distributed under the same license as the PACKAGE package.\n"
           "# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n"
           "#\n"
           "#, fuzzy\n"
           "msgid \"\"\n"
           "msgstr \"\"\n"
           "\"Project-Id-Version: PACKAGE VERSION\\n\"\n"
           "\"Report-Msgid-Bugs-To: fcitx-dev@googlegroups.com\\n\"\n"
           "\"POT-Creation-Date: 2014-06-14 15:01+0200\\n\"\n"
           "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
           "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n"
           "\"Language-Team: LANGUAGE <LL@li.org>\\n\"\n"
           "\"Language: LANG\\n\"\n"
           "\"MIME-Version: 1.0\\n\"\n"
           "\"Content-Type: text/plain; charset=utf-8\\n\"\n"
           "\"Content-Transfer-Encoding: 8bit\\n\"\n"
           "\n");
}



void print_pot_string(const char* str)
{
    fprintf(fout, "msgid \"%s\"\n"
           "msgstr \"\"\n"
           "\n", str);
}

typedef struct {
    FcitxConfiguration* rootConfig;
    FcitxStringHashSet* stringSet;
} print_pot_definition_context;

void print_pot_attribute(FcitxConfiguration* config, const char* path, void* userData)
{
    FCITX_UNUSED(path);
    FcitxStringHashSet* stringSet = userData;
    fcitx_string_hashset_insert(stringSet, fcitx_configuration_get_name(config));
    const char* description = fcitx_configuration_get_value_by_path(config, "Description");
    if (description) {
        fcitx_string_hashset_insert(stringSet, description);
    }
    const char* LongDescription = fcitx_configuration_get_value_by_path(config, "LongDescription");
    if (LongDescription) {
        fcitx_string_hashset_insert(stringSet, LongDescription);
    }

    const char* type = fcitx_configuration_get_value_by_path(config, "Type");
    if (strcmp(type, "Enum") == 0) {
        const char* enumCountString = fcitx_configuration_get_value_by_path(config, "EnumCount");
        int enumCount = atoi(enumCountString);
        for (int i = 0; i < enumCount; i++) {
            char buf[64];
            sprintf(buf, "Enum%d", i);
            // get Enum0, Enum1.. etc
            const char* enumString = fcitx_configuration_get_value_by_path(config, buf);
            fcitx_string_hashset_insert(stringSet, enumString);
        }
    }
}

bool print_pot_definition(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(data);
    print_pot_definition_context *context = userData;
    FcitxConfiguration* config = context->rootConfig;
    fcitx_string_hashset_insert(context->stringSet, key);
    fcitx_configuration_foreach(config, key, false, "", print_pot_attribute, context->stringSet);
    return false;
}

bool print_pot_strings_foreach(const char* key, size_t keyLen, void** data, void* userData)
{
    FCITX_UNUSED(keyLen);
    FCITX_UNUSED(data);
    FCITX_UNUSED(userData);
    print_pot_string(key);
    return false;
}

void compile_to_pot(FcitxConfiguration* config, FcitxDescription* desc)
{
    print_pot_header();
    //[Group/Name]
    //Description
    //LongDescription
    FcitxStringHashSet* stringSet = fcitx_string_hashset_new();
    print_pot_definition_context context;
    context.stringSet = stringSet;
    context.rootConfig = config;

    fcitx_dict_foreach(desc->structs, print_pot_definition, &context);

    fcitx_dict_foreach(stringSet, print_pot_strings_foreach, NULL);
    fcitx_string_hashset_free(stringSet);
}
