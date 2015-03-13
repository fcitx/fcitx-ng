/*
 * Copyright (C) 2015~2015 by CSSlayer
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

#include <stdlib.h>
#include <locale.h>
#include "utils.h"

FCITX_EXPORT_API
FcitxI18NString* fcitx_i18n_string_new()
{
    FcitxI18NString* dict = fcitx_dict_new(free);
    return dict;
}

FCITX_EXPORT_API
const char* fcitx_i18n_string_match(FcitxI18NString* s, const char* locale)
{
    if (!locale) {
        locale = setlocale(LC_MESSAGES, NULL);
    }
    if (!locale) {
        locale = "";
    }
    // regex
    // ^(?P<language>[^_.@[:space:]]+)
    // (_(?P<territory>[[:upper:]]+))?
    // (\\.(?P<codeset>[-_0-9a-zA-Z]+))?
    // (@(?P<modifier>[[:ascii:]]+))?$
    //
    // check locale format.
    // [language]_[country].[encoding]@modifier
    // we don't want too large locale to match.
    char normalizedLocale[64 + 1];
    size_t languageLength = 0;
    size_t territoryLength = 0;
    size_t modifierLength = 0;
    bool failed = false;
    size_t i = 0;
    size_t j = 0;
    do {
        while (i < sizeof(normalizedLocale) - 1
            && locale[i] != '\0'
            && !fcitx_utils_isspace(locale[i])
            && locale[i] != '_'
            && locale[i] != '.'
            && locale[i] != '@') {
            normalizedLocale[j] = locale[i];
            i++;
            j++;
        }

        if (i == 0 || i == sizeof(normalizedLocale) - 1) {
            failed = true;
            break;
        }
        languageLength = j;

        if (locale[i] == '_') {
            normalizedLocale[j] = '_';
            i++;
            j++;
            while (i < sizeof(normalizedLocale) - 1
                && locale[i] != '\0'
                && fcitx_utils_isupper(locale[i])) {
                normalizedLocale[j] = locale[i];
                i++;
                j++;
            }

            if (i == sizeof(normalizedLocale) - 1) {
                failed = true;
                break;
            }

            territoryLength = j;
        }

        if (locale[i] == '.') {
            // encoding is useless for us
            i++;
            while (i < sizeof(normalizedLocale) - 1
                && locale[i] != '\0'
                && (fcitx_utils_isupper(locale[i])
                 || fcitx_utils_islower(locale[i])
                 || fcitx_utils_isdigit(locale[i])
                 || locale[i] == '_'
                 || locale[i] == '-')) {
                i++;
            }
        }

        if (locale[i] == '@') {
            normalizedLocale[j] = '@';
            i++;
            j++;
            while (i < sizeof(normalizedLocale) - 1
                && locale[i] != '\0') {
                normalizedLocale[j] = locale[i];
                i++;
                j++;
            }

            if (i == sizeof(normalizedLocale) - 1) {
                failed = true;
                break;
            }

            modifierLength = j;
        }
    } while(0);

    if (failed) {
        normalizedLocale[0] = '\0';
        modifierLength = territoryLength = languageLength = 0;
    }

    const char* result = NULL;
    do {
        if (modifierLength && fcitx_dict_lookup(s, normalizedLocale, modifierLength, &result)) {
            break;
        }
        if (territoryLength && fcitx_dict_lookup(s, normalizedLocale, territoryLength, &result)) {
            break;
        }
        if (fcitx_dict_lookup(s, normalizedLocale, languageLength, &result)) {
            break;
        }
        fcitx_dict_lookup_by_str(s, "", &result);
    } while(0);

    return result;
}
