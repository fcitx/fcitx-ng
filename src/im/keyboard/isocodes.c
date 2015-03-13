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

#include <libxml/parser.h>

#include <string.h>

#include <fcitx-utils/utils.h>
#include "isocodes.h"

#define XMLCHAR_CAST (const char*)

static void IsoCodes639HandlerStartElement(void *ctx,
                                           const xmlChar *name,
                                           const xmlChar **atts);
static void IsoCodes3166HandlerStartElement(void *ctx,
                                            const xmlChar *name,
                                            const xmlChar **atts);
static void FcitxIsoCodes639EntryFree(FcitxIsoCodes639Entry* entry);
static void FcitxIsoCodes3166EntryFree(FcitxIsoCodes3166Entry* entry);

FcitxIsoCodes* FcitxXkbReadIsoCodes(const char* iso639, const char* iso3166)
{
    xmlSAXHandler handle;
    memset(&handle, 0, sizeof(xmlSAXHandler));

    xmlInitParser();

    FcitxIsoCodes* isocodes = (FcitxIsoCodes*) fcitx_utils_malloc0(sizeof(FcitxIsoCodes));

    handle.startElement = IsoCodes639HandlerStartElement;
    xmlSAXUserParseFile(&handle, isocodes, iso639);
    handle.startElement = IsoCodes3166HandlerStartElement;
    xmlSAXUserParseFile(&handle, isocodes, iso3166);

    /* DO NOT Call xmlCleanupParser() */

    return isocodes;
}

static void IsoCodes639HandlerStartElement(void *ctx,
                                           const xmlChar *name,
                                           const xmlChar **atts)
{
    FcitxIsoCodes* isocodes = ctx;
    if (strcmp(XMLCHAR_CAST name, "iso_639_entry") == 0) {
        FcitxIsoCodes639Entry* entry = fcitx_utils_malloc0(sizeof(FcitxIsoCodes639Entry));
        int i = 0;
        while(atts && atts[i*2] != 0) {
            if (strcmp(XMLCHAR_CAST atts[i * 2], "iso_639_2B_code") == 0)
                entry->iso_639_2B_code = fcitx_utils_strdup(XMLCHAR_CAST atts[i * 2 + 1]);
            else if (strcmp(XMLCHAR_CAST atts[i * 2], "iso_639_2T_code") == 0)
                entry->iso_639_2T_code = fcitx_utils_strdup(XMLCHAR_CAST atts[i * 2 + 1]);
            else if (strcmp(XMLCHAR_CAST atts[i * 2], "iso_639_1_code") == 0)
                entry->iso_639_1_code = fcitx_utils_strdup(XMLCHAR_CAST atts[i * 2 + 1]);
            else if (strcmp(XMLCHAR_CAST atts[i * 2], "name") == 0)
                entry->name = fcitx_utils_strdup(XMLCHAR_CAST atts[i * 2 + 1]);
            i++;
        }
        if (!entry->iso_639_2B_code || !entry->iso_639_2T_code || !entry->name)
            FcitxIsoCodes639EntryFree(entry);
        else {
            HASH_ADD_KEYPTR(hh1, isocodes->iso6392B, entry->iso_639_2B_code, strlen(entry->iso_639_2B_code), entry);
            HASH_ADD_KEYPTR(hh2, isocodes->iso6392T, entry->iso_639_2T_code, strlen(entry->iso_639_2T_code), entry);
        }
    }
}

void FcitxIsoCodes639EntryFree(FcitxIsoCodes639Entry* entry)
{
    fcitx_utils_free(entry->iso_639_1_code);
    fcitx_utils_free(entry->iso_639_2B_code);
    fcitx_utils_free(entry->iso_639_2T_code);
    fcitx_utils_free(entry->name);
    free(entry);
}

void FcitxIsoCodes3166EntryFree(FcitxIsoCodes3166Entry* entry)
{
    fcitx_utils_free(entry->alpha_2_code);
    fcitx_utils_free(entry->name);
    free(entry);
}


static void IsoCodes3166HandlerStartElement(void *ctx,
                                           const xmlChar *name,
                                           const xmlChar **atts)
{
    FcitxIsoCodes* isocodes = ctx;
    if (strcmp(XMLCHAR_CAST name, "iso_3166_entry") == 0) {
        FcitxIsoCodes3166Entry* entry = fcitx_utils_malloc0(sizeof(FcitxIsoCodes3166Entry));
        int i = 0;
        while(atts && atts[i*2] != 0) {
            if (strcmp(XMLCHAR_CAST atts[i * 2], "alpha_2_code") == 0)
                entry->alpha_2_code = fcitx_utils_strdup(XMLCHAR_CAST atts[i * 2 + 1]);
            else if (strcmp(XMLCHAR_CAST atts[i * 2], "name") == 0)
                entry->name = fcitx_utils_strdup(XMLCHAR_CAST atts[i * 2 + 1]);
            i++;
        }
        if (!entry->name || !entry->alpha_2_code)
            FcitxIsoCodes3166EntryFree(entry);
        else
            HASH_ADD_KEYPTR(hh, isocodes->iso3166, entry->alpha_2_code, strlen(entry->alpha_2_code), entry);
    }
}

FcitxIsoCodes639Entry* FcitxIsoCodesGetEntry(FcitxIsoCodes* isocodes, const char* lang)
{
    FcitxIsoCodes639Entry *entry = NULL;
    HASH_FIND(hh1,isocodes->iso6392B,lang,strlen(lang),entry);
    if (!entry) {
        HASH_FIND(hh2,isocodes->iso6392T,lang,strlen(lang),entry);
    }
    return entry;
}

void FcitxIsoCodesFree(FcitxIsoCodes* isocodes)
{
    FcitxIsoCodes639Entry* isocodes639 = isocodes->iso6392B;
    while (isocodes639) {
        FcitxIsoCodes639Entry* curisocodes639 = isocodes639;
        HASH_DELETE(hh1, isocodes639, curisocodes639);
    }

    isocodes639 = isocodes->iso6392T;
    while (isocodes639) {
        FcitxIsoCodes639Entry* curisocodes639 = isocodes639;
        HASH_DELETE(hh2, isocodes639, curisocodes639);
        FcitxIsoCodes639EntryFree(curisocodes639);
    }
    FcitxIsoCodes3166Entry* isocodes3166 = isocodes->iso3166;
    while (isocodes3166) {
        FcitxIsoCodes3166Entry* curisocodes3166 = isocodes3166;
        HASH_DEL(isocodes3166, curisocodes3166);
        FcitxIsoCodes3166EntryFree(curisocodes3166);
    }

    free(isocodes);
}

const char* FindBestLanguage(FcitxIsoCodes* isocodes, const char* hint, UT_array* languages)
{
    const char* bestLang = NULL;

    /* score:
     * 1 -> first one
     * 2 -> match 2
     * 3 -> match three
     */
    FcitxIsoCodes639Entry* bestEntry = NULL;
    int bestScore = 0;
    utarray_foreach(plang, languages, char*) {
        FcitxIsoCodes639Entry* entry = FcitxIsoCodesGetEntry(isocodes, *plang);
        if (!entry) {
            continue;
        }

        const char* lang = entry->iso_639_1_code;
        if (!lang) {
            lang = entry->iso_639_2T_code;
        }

        if (!lang) {
            lang = entry->iso_639_2B_code;
        }

        if (!lang) {
            continue;
        }

        size_t len = strlen(lang);
        if (len != 2 && len != 3) {
            continue;
        }

        int score = 1;
        while (len >= 2) {
            if (strncasecmp(hint, lang, len) == 0) {
                score = len;
                break;
            }

            len --;
        }

        if (bestScore < score) {
            bestEntry = entry;
            bestScore = score;
        }
    }
    if (bestEntry) {
        bestLang = bestEntry->iso_639_1_code;
        if (!bestLang) {
            bestLang = bestEntry->iso_639_2T_code;
        }
        if (!bestLang) {
            bestLang = bestEntry->iso_639_2B_code;
        }
    }
    return bestLang;
}
