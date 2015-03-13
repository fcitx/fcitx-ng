/*
 * Copyright (C) 2012~2015 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "fcitx/common.h"
#include <libintl.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "fcitx/ime.h"
#include "isocodes.h"
#include "rules.h"
#include "module/xcb/fcitx-xcb.h"

#define XKB_RULES_XML_FILE XKEYBOARDCONFIG_XKBBASE "/rules/evdev.xml"

typedef struct _FcitxKeyboard
{
    FcitxAddonManager* manager;
    FcitxStandardPath* standardPath;
    FcitxInputMethodManager* immanager;
    bool enUSRegistered;
} FcitxKeyboard;

typedef struct _FcitxKeyboardLayout
{
    char* layoutString;
    char* variantString;
    FcitxKeyboard* owner;
} FcitxKeyboardLayout;

static void* fcitx_keyboard_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_keyboard_destroy(void* data);
static bool fcitx_keyboard_handle_event(void* data, const FcitxIMEvent* event);
static char* FcitxXkbFindXkbRulesFile(FcitxKeyboard* self);

static void FcitxKeyboardLayoutCreate(FcitxKeyboard* keyboard,
                                      const char* name,
                                      const char* langCode,
                                      const char* layoutString,
                                      const char* variantString
                                     );

FCITX_DEFINE_ADDON(fcitx_keyboard, inputmethod, FcitxAddonAPIInputMethod) = {
    .common = {
        .init = fcitx_keyboard_init,
        .destroy = fcitx_keyboard_destroy
    }
};

void* fcitx_keyboard_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(manager);
    FCITX_UNUSED(config);
    FcitxKeyboard* keyboard = fcitx_utils_new(FcitxKeyboard);
    char* localepath = fcitx_utils_get_fcitx_path("localedir");
    bindtextdomain("xkeyboard-config", localepath);
    bind_textdomain_codeset("xkeyboard-config", "UTF-8");
    free(localepath);

    keyboard->manager = manager;
    keyboard->standardPath = fcitx_standard_path_ref(fcitx_addon_manager_get_standard_path(manager));
    keyboard->immanager = fcitx_input_method_manager_ref(fcitx_addon_manager_get_property(manager, "immanager"));

    FcitxIsoCodes* isoCodes = NULL;
    FcitxXkbRules* rules = NULL;
#ifdef ENABLE_LIBXML2
    char* name = FcitxXkbFindXkbRulesFile(keyboard);
    if (name) {
        rules = FcitxXkbReadRules(name);
        isoCodes = FcitxXkbReadIsoCodes(ISOCODES_ISO639_XML, ISOCODES_ISO3166_XML);
    }
#endif

    if (rules && utarray_len(rules->layoutInfos) && isoCodes) {
        FcitxStandardPathFile file = fcitx_standard_path_create_tempfile(keyboard->standardPath, FSPT_Config, "cached_layout.XXXXXX");

        utarray_foreach(layoutInfo, rules->layoutInfos, FcitxXkbLayoutInfo) {
            {
                const char* lang = FindBestLanguage(isoCodes, layoutInfo->description, layoutInfo->languages);
                char *description;
                fcitx_utils_alloc_cat_str(description, _("Keyboard"), " - ",
                                          dgettext("xkeyboard-config",
                                                   layoutInfo->description));
                FcitxKeyboardLayoutCreate(keyboard, description,
                                          lang, layoutInfo->name, NULL);
                free(description);
                if (file.fp) {
                    fprintf(file.fp, "%s:%s:%s\n", layoutInfo->description, layoutInfo->name, lang ? lang : "");
                }
            }
            FcitxXkbVariantInfo* variantInfo;
            for (variantInfo = (FcitxXkbVariantInfo*) utarray_front(layoutInfo->variantInfos);
                variantInfo != NULL;
                variantInfo = (FcitxXkbVariantInfo*) utarray_next(layoutInfo->variantInfos, variantInfo))
            {
                const char* lang = FindBestLanguage(isoCodes, layoutInfo->description,
                                                    utarray_len(variantInfo->languages) > 0 ? variantInfo->languages : layoutInfo->languages);
                char *description;
                fcitx_utils_alloc_cat_str(description, _("Keyboard"), " - ",
                                          dgettext("xkeyboard-config",
                                                   layoutInfo->description),
                                          " - ",
                                          dgettext("xkeyboard-config",
                                                   variantInfo->description));
                FcitxKeyboardLayoutCreate(keyboard, description, lang,
                                          layoutInfo->name, variantInfo->name);
                if (file.fp) {
                    fprintf(file.fp, "%s:%s:%s:%s:%s\n",
                            layoutInfo->description, layoutInfo->name,
                            variantInfo->description, variantInfo->name,
                            lang ? lang : "");
                }
                free(description);
            }
        }

        if (file.fp) {
            fclose(file.fp);
            char* path = fcitx_utils_strdup(file.path);
            path[strlen(path) - 7] = '\0';
            if (rename(file.path, path) < 0) {
                unlink(file.path);
            }

            free(file.path);
            free(path);
        }
    }
    else {
        FcitxStandardPathFile* file = fcitx_standard_path_locate(keyboard->standardPath, FSPT_Config, "cached_layout", FSPFT_Writable);
        if (file) {
            char *buf = NULL, *buf1 = NULL;
            size_t len = 0;
            while (getline(&buf, &len, file->fp) != -1) {
                fcitx_utils_free(buf1);
                buf1 = fcitx_utils_trim(buf);
                FcitxStringList* list = fcitx_utils_string_split(buf1, ":");
                if (utarray_len(list) == 3) {
                    char** layoutDescription = (char**)utarray_eltptr(list, 0);
                    char** layoutName = (char**)utarray_eltptr(list, 1);
                    char** layoutLang = (char**)utarray_eltptr(list, 2);
                    char *description;
                    fcitx_utils_alloc_cat_str(description, _("Keyboard"), " - ",
                                              dgettext("xkeyboard-config",
                                                     *layoutDescription), " ", _("(Unavailable)"));
                    FcitxKeyboardLayoutCreate(keyboard, description,
                                            *layoutLang, *layoutName, NULL);
                }
                else if (utarray_len(list) == 5) {
                    char** layoutDescription = (char**)utarray_eltptr(list, 0);
                    char** layoutName = (char**)utarray_eltptr(list, 1);
                    char** variantDescription = (char**)utarray_eltptr(list, 2);
                    char** variantName = (char**)utarray_eltptr(list, 3);
                    char** variantLang = (char**)utarray_eltptr(list, 4);
                    char *description;
                    fcitx_utils_alloc_cat_str(description, _("Keyboard"), " - ",
                                              dgettext("xkeyboard-config",
                                                       *layoutDescription),
                                              " - ",
                                              dgettext("xkeyboard-config",
                                                       *variantDescription),
                                              " ", _("(Unavailable)"));
                    FcitxKeyboardLayoutCreate(keyboard, description, *variantLang,
                                              *layoutName, *variantName);
                }
                fcitx_utils_string_list_free(list);
            }

            fcitx_utils_free(buf);
            fcitx_utils_free(buf1);

            fcitx_standard_path_file_close(file);
        }
        else
        {
            FcitxKeyboardLayoutCreate(keyboard, _("Keyboard"), "en", "us", NULL);
        }
    }

    /* always have en us is better */
    if (!keyboard->enUSRegistered)
        FcitxKeyboardLayoutCreate(keyboard, _("Keyboard"), "en", "us", NULL);

    FcitxIsoCodesFree(isoCodes);
    FcitxXkbRulesFree(rules);

    return keyboard;
}

void fcitx_keyboard_destroy(void* data)
{
    FcitxKeyboard* keyboard = data;
    free(keyboard);
}

bool fcitx_keyboard_handle_event(void* data, const FcitxIMEvent* event)
{
    switch (event->type) {
        case IET_Init:
            break;
        case IET_Keyboard:
            break;
        case IET_Reset:
            break;
        case IET_SurroundingTextUpdated:
            break;
    }
    return false;
}

void FcitxKeyboardLayoutCreate(FcitxKeyboard* keyboard, const char* name, const char* langCode, const char* layoutString, const char* variantString)
{
    FcitxKeyboardLayout* layout = fcitx_utils_new(FcitxKeyboardLayout);

    layout->layoutString = fcitx_utils_strdup(layoutString);
    if (variantString) {
        layout->variantString = fcitx_utils_strdup(variantString);
    }
    layout->owner = keyboard;

    if (fcitx_utils_strcmp0(langCode, "en") == 0
        && fcitx_utils_strcmp0(layoutString, "us") == 0
        && fcitx_utils_strcmp0(variantString, NULL) == 0) {
        keyboard->enUSRegistered = true;
    }

    char *uniqueName;
    if (variantString) {
        fcitx_utils_alloc_cat_str(uniqueName, "fcitx-keyboard-", layoutString,
                                  "-", variantString);
    } else {
        fcitx_utils_alloc_cat_str(uniqueName, "fcitx-keyboard-", layoutString);
    }

    fcitx_input_method_manager_register(keyboard->immanager,
                                        layout,
                                        uniqueName,
                                        name,
                                        "kbd",
                                        fcitx_keyboard_handle_event,
                                        0,
                                        langCode);

    free(uniqueName);
}


static char* FcitxXkbFindXkbRulesFile(FcitxKeyboard* self)
{
    char *rulesFile = NULL;
    char *rulesName = fcitx_xcb_invoke_get_xkb_rules_names(self->manager, NULL);

    if (rulesName) {
        if (rulesName[0] == '/') {
            fcitx_utils_alloc_cat_str(rulesFile, rulesName, ".xml");
        } else {
            const char* base = XKEYBOARDCONFIG_XKBBASE;
            fcitx_utils_alloc_cat_str(rulesFile, base,
                                      "/rules/", rulesName, ".xml");
        }
        free(rulesName);
    } else {
        return fcitx_utils_strdup(XKB_RULES_XML_FILE);
    }
    return rulesFile;
}
