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

#include <libxml/parser.h>

#include "rules.h"
#define XMLCHAR_CAST (const char*)

static void RulesHandlerStartElement(void *ctx, const xmlChar *name,
                                     const xmlChar **atts);
static void RulesHandlerEndElement(void *ctx, const xmlChar *name);
static void RulesHandlerCharacters(void *ctx, const xmlChar *ch, int len);
static void FcitxXkbLayoutInfoInit(void* arg);
static void FcitxXkbVariantInfoInit(void* arg);
static void FcitxXkbModelInfoInit(void* arg);
static void FcitxXkbOptionGroupInfoInit(void* arg);
static void FcitxXkbOptionInfoInit(void* arg);
static void FcitxXkbLayoutInfoCopy(void* dst, const void* src);
static void FcitxXkbVariantInfoCopy(void* dst, const void* src);
static void FcitxXkbModelInfoCopy(void* dst, const void* src);
static void FcitxXkbOptionGroupInfoCopy(void* dst, const void* src);
static void FcitxXkbOptionInfoCopy(void* dst, const void* src);
static void FcitxXkbLayoutInfoFree(void* arg);
static void FcitxXkbVariantInfoFree(void* arg);
static void FcitxXkbModelInfoFree(void* arg);
static void FcitxXkbOptionGroupInfoFree(void* arg);
static void FcitxXkbOptionInfoFree(void* arg);
static inline FcitxXkbLayoutInfo* FindByName(FcitxXkbRules* rules, const char* name);
static void MergeRules(FcitxXkbRules* rules, FcitxXkbRules* rulesextra);

static const UT_icd layout_icd = {
    sizeof(FcitxXkbLayoutInfo), FcitxXkbLayoutInfoInit,
    FcitxXkbLayoutInfoCopy, FcitxXkbLayoutInfoFree
};
static const UT_icd variant_icd = {
    sizeof(FcitxXkbVariantInfo), FcitxXkbVariantInfoInit,
    FcitxXkbVariantInfoCopy, FcitxXkbVariantInfoFree
};
static const UT_icd model_icd = {
    sizeof(FcitxXkbModelInfo), FcitxXkbModelInfoInit,
    FcitxXkbModelInfoCopy, FcitxXkbModelInfoFree
};
static const UT_icd option_group_icd = {
    sizeof(FcitxXkbOptionGroupInfo), FcitxXkbOptionGroupInfoInit,
    FcitxXkbOptionGroupInfoCopy, FcitxXkbOptionGroupInfoFree
};
static const UT_icd option_icd = {
    sizeof(FcitxXkbOptionInfo), FcitxXkbOptionInfoInit,
    FcitxXkbOptionInfoCopy, FcitxXkbOptionInfoFree
};
static const UT_icd ptr_icd = {
    sizeof(void*), NULL, NULL, NULL
};

FcitxXkbRules* FcitxXkbReadRules(const char* file)
{
    xmlSAXHandler handle;
    memset(&handle, 0, sizeof(xmlSAXHandler));
    handle.startElement = RulesHandlerStartElement;
    handle.endElement = RulesHandlerEndElement;
    handle.characters = RulesHandlerCharacters;

    xmlInitParser();

    FcitxXkbRules* rules = fcitx_utils_new(FcitxXkbRules);
    rules->layoutInfos = utarray_new(&layout_icd);
    rules->modelInfos = utarray_new(&model_icd);
    rules->optionGroupInfos = utarray_new(&option_group_icd);

    FcitxXkbRulesHandler ruleshandler;
    ruleshandler.rules = rules;
    ruleshandler.path = fcitx_utils_string_list_new();
    ruleshandler.fromExtra = false;

    xmlSAXUserParseFile(&handle, &ruleshandler, file);
    utarray_free(ruleshandler.path);

    size_t extra_len = strlen(file) - strlen(".xml");
    if (strcmp(file + extra_len, ".xml") == 0) {
        char extrafile[extra_len + strlen(".extras.xml") + 1];
        memcpy(extrafile, file, extra_len);
        memcpy(extrafile + extra_len, ".extras.xml", sizeof(".extras.xml"));
        FcitxXkbRules *rulesextra = fcitx_utils_new(FcitxXkbRules);
        rulesextra->layoutInfos = utarray_new(&layout_icd);
        rulesextra->modelInfos = utarray_new(&model_icd);
        rulesextra->optionGroupInfos = utarray_new(&option_group_icd);
        ruleshandler.rules = rulesextra;
        ruleshandler.path = fcitx_utils_string_list_new();
        xmlSAXUserParseFile(&handle, &ruleshandler, extrafile);
        utarray_free(ruleshandler.path);
        MergeRules(rules, rulesextra);
    }

    /* DO NOT Call xmlCleanupParser() */

    return rules;
}

void FcitxXkbRulesFree(FcitxXkbRules* rules)
{
    if (!rules)
        return;

    utarray_free(rules->layoutInfos);
    utarray_free(rules->modelInfos);
    utarray_free(rules->optionGroupInfos);

    fcitx_utils_free(rules->version);
    free(rules);
}

void MergeRules(FcitxXkbRules* rules, FcitxXkbRules* rulesextra)
{
    utarray_concat(rules->modelInfos, rulesextra->modelInfos);
    utarray_concat(rules->optionGroupInfos, rulesextra->optionGroupInfos);

    FcitxXkbLayoutInfo* layoutInfo;
    UT_array toAdd;
    utarray_init(&toAdd, &ptr_icd);
    for (layoutInfo = (FcitxXkbLayoutInfo*) utarray_front(rulesextra->layoutInfos);
         layoutInfo != NULL;
         layoutInfo = (FcitxXkbLayoutInfo*) utarray_next(rulesextra->layoutInfos, layoutInfo))
    {
        FcitxXkbLayoutInfo* l = FindByName(rules, layoutInfo->name);
        if (l) {
            utarray_concat(l->languages, layoutInfo->languages);
            utarray_concat(l->variantInfos, layoutInfo->variantInfos);
        }
        else
            utarray_push_back(&toAdd, &layoutInfo);
    }

    unsigned int i;
    for(i = 0;i < utarray_len(&toAdd);i++) {
        FcitxXkbLayoutInfo* p = *(FcitxXkbLayoutInfo**)utarray_eltptr(&toAdd, i);
        utarray_push_back(rules->layoutInfos, p);
    }

    utarray_done(&toAdd);
    FcitxXkbRulesFree(rulesextra);
}


char* FcitxXkbRulesToReadableString(FcitxXkbRules* rules)
{
    FcitxXkbLayoutInfo* layoutInfo;
    FcitxXkbModelInfo* modelInfo;
    FcitxXkbVariantInfo* variantInfo;
    FcitxXkbOptionInfo* optionInfo;
    FcitxXkbOptionGroupInfo* optionGroupInfo;

    UT_array* list = fcitx_utils_string_list_new();

    fcitx_utils_string_list_printf_append(list, "Version: %s", rules->version);

    for (layoutInfo = (FcitxXkbLayoutInfo*) utarray_front(rules->layoutInfos);
         layoutInfo != NULL;
         layoutInfo = (FcitxXkbLayoutInfo*) utarray_next(rules->layoutInfos, layoutInfo))
    {
        fcitx_utils_string_list_printf_append(list, "\tLayout Name: %s", layoutInfo->name);
        fcitx_utils_string_list_printf_append(list, "\tLayout Description: %s", layoutInfo->description);
        char* languages = fcitx_utils_string_list_join(layoutInfo->languages, ',');
        fcitx_utils_string_list_printf_append(list, "\tLayout Languages: %s", languages);
        free(languages);
        for (variantInfo = (FcitxXkbVariantInfo*) utarray_front(layoutInfo->variantInfos);
             variantInfo != NULL;
             variantInfo = (FcitxXkbVariantInfo*) utarray_next(layoutInfo->variantInfos, variantInfo))
        {
            fcitx_utils_string_list_printf_append(list, "\t\tVariant Name: %s", variantInfo->name);
            fcitx_utils_string_list_printf_append(list, "\t\tVariant Description: %s", variantInfo->description);
            char* languages = fcitx_utils_string_list_join(variantInfo->languages, ',');
            fcitx_utils_string_list_printf_append(list, "\t\tVariant Languages: %s", languages);
            free(languages);
        }
    }

    for (modelInfo = (FcitxXkbModelInfo*) utarray_front(rules->modelInfos);
         modelInfo != NULL;
         modelInfo = (FcitxXkbModelInfo*) utarray_next(rules->modelInfos, modelInfo))
    {
        fcitx_utils_string_list_printf_append(list, "\tModel Name: %s", modelInfo->name);
        fcitx_utils_string_list_printf_append(list, "\tModel Description: %s", modelInfo->description);
        fcitx_utils_string_list_printf_append(list, "\tModel Vendor: %s", modelInfo->vendor);
    }

    for (optionGroupInfo = (FcitxXkbOptionGroupInfo*) utarray_front(rules->optionGroupInfos);
         optionGroupInfo != NULL;
         optionGroupInfo = (FcitxXkbOptionGroupInfo*) utarray_next(rules->optionGroupInfos, optionGroupInfo))
    {
        fcitx_utils_string_list_printf_append(list, "\tOption Group Name: %s", optionGroupInfo->name);
        fcitx_utils_string_list_printf_append(list, "\tOption Group Description: %s", optionGroupInfo->description);
        fcitx_utils_string_list_printf_append(list, "\tOption Group Exclusive: %d", optionGroupInfo->exclusive);
        for (optionInfo = (FcitxXkbOptionInfo*) utarray_front(optionGroupInfo->optionInfos);
             optionInfo != NULL;
             optionInfo = (FcitxXkbOptionInfo*) utarray_next(optionGroupInfo->optionInfos, optionInfo))
        {
            fcitx_utils_string_list_printf_append(list, "\t\tOption Name: %s", optionInfo->name);
            fcitx_utils_string_list_printf_append(list, "\t\tOption Description: %s", optionInfo->description);
        }
    }

    char* result = fcitx_utils_string_list_join(list, '\n');
    utarray_free(list);
    return result;
}

static inline
FcitxXkbLayoutInfo* FindByName(FcitxXkbRules* rules, const char* name) {
    FcitxXkbLayoutInfo* layoutInfo;
    for (layoutInfo = (FcitxXkbLayoutInfo*) utarray_front(rules->layoutInfos);
         layoutInfo != NULL;
         layoutInfo = (FcitxXkbLayoutInfo*) utarray_next(rules->layoutInfos, layoutInfo))
    {
        if (strcmp(layoutInfo->name, name) == 0)
            break;
    }
    return layoutInfo;
}

/* code borrow from kde-workspace/kcontrol/keyboard/xkb_rules.cpp */
void RulesHandlerStartElement(void *ctx, const xmlChar *name,
                              const xmlChar **atts)
{
    FcitxXkbRulesHandler* ruleshandler = (FcitxXkbRulesHandler*) ctx;
    FcitxXkbRules* rules = ruleshandler->rules;
    utarray_push_back(ruleshandler->path, &name);

    char* strPath = fcitx_utils_string_list_join(ruleshandler->path, '/');
    if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/configItem") ) {
        utarray_extend_back(rules->layoutInfos);
    }
    else if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/variantList/variant") ) {
        FcitxXkbLayoutInfo* layoutInfo = (FcitxXkbLayoutInfo*) utarray_back(rules->layoutInfos);
        utarray_extend_back(layoutInfo->variantInfos);
    }
    else if ( fcitx_utils_string_ends_with(strPath, "modelList/model") ) {
        utarray_extend_back(rules->modelInfos);
    }
    else if ( fcitx_utils_string_ends_with(strPath, "optionList/group") ) {
        utarray_extend_back(rules->optionGroupInfos);
        FcitxXkbOptionGroupInfo* optionGroupInfo = (FcitxXkbOptionGroupInfo*) utarray_back(rules->optionGroupInfos);
        int i = 0;
        while(atts && atts[i*2] != 0) {
            if (strcmp(XMLCHAR_CAST atts[i*2], "allowMultipleSelection") == 0) {
                optionGroupInfo->exclusive = (strcmp(XMLCHAR_CAST atts[i*2 + 1], "true") != 0);
            }
            i++;
        }
    }
    else if ( fcitx_utils_string_ends_with(strPath, "optionList/group/option") ) {
        FcitxXkbOptionGroupInfo* optionGroupInfo = (FcitxXkbOptionGroupInfo*) utarray_back(rules->optionGroupInfos);
        utarray_extend_back(optionGroupInfo->optionInfos);
    }
    else if ( strcmp(strPath, "xkbConfigRegistry") == 0 ) {
        int i = 0;
        while(atts && atts[i*2] != 0) {
            if (strcmp(XMLCHAR_CAST atts[i*2], "version") == 0 && strlen(XMLCHAR_CAST atts[i*2 + 1]) != 0) {
                rules->version = fcitx_utils_strdup(XMLCHAR_CAST atts[i*2 + 1]);
            }
            i++;
        }
    }
    free(strPath);
}

void RulesHandlerEndElement(void *ctx, const xmlChar *name)
{
    FCITX_UNUSED(name);
    FcitxXkbRulesHandler* ruleshandler = (FcitxXkbRulesHandler*) ctx;
    utarray_pop_back(ruleshandler->path);
}

void RulesHandlerCharacters(void *ctx,
                const xmlChar *ch,
                int len)
{
    FcitxXkbRulesHandler* ruleshandler = (FcitxXkbRulesHandler*) ctx;
    FcitxXkbRules* rules = ruleshandler->rules;
    char* temp = strndup(XMLCHAR_CAST ch, len);
    char* trimmed = fcitx_utils_trim(temp);
    free(temp);
    if ( strlen(trimmed) != 0 ) {
        char* strPath = fcitx_utils_string_list_join(ruleshandler->path, '/');
        FcitxXkbLayoutInfo* layoutInfo = (FcitxXkbLayoutInfo*) utarray_back(rules->layoutInfos);
        FcitxXkbModelInfo* modelInfo = (FcitxXkbModelInfo*) utarray_back(rules->modelInfos);
        FcitxXkbOptionGroupInfo* optionGroupInfo = (FcitxXkbOptionGroupInfo*) utarray_back(rules->optionGroupInfos);
        if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/configItem/name") ) {
            if ( layoutInfo != NULL )
                layoutInfo->name = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/configItem/description") ) {
            layoutInfo->description = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/configItem/languageList/iso639Id") ) {
            utarray_push_back(layoutInfo->languages, &trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/variantList/variant/configItem/name") ) {
            FcitxXkbVariantInfo* variantInfo = (FcitxXkbVariantInfo*) utarray_back(layoutInfo->variantInfos);
            variantInfo->name = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/variantList/variant/configItem/description") ) {
            FcitxXkbVariantInfo* variantInfo = (FcitxXkbVariantInfo*) utarray_back(layoutInfo->variantInfos);
            fcitx_utils_free(variantInfo->description);
            variantInfo->description = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "layoutList/layout/variantList/variant/configItem/languageList/iso639Id") ) {
            FcitxXkbVariantInfo* variantInfo = (FcitxXkbVariantInfo*) utarray_back(layoutInfo->variantInfos);
            utarray_push_back(variantInfo->languages, &trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "modelList/model/configItem/name") ) {
            modelInfo->name = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "modelList/model/configItem/description") ) {
            modelInfo->description = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "modelList/model/configItem/vendor") ) {
            modelInfo->vendor = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "optionList/group/configItem/name") ) {
            optionGroupInfo->name = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "optionList/group/configItem/description") ) {
            optionGroupInfo->description = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "optionList/group/option/configItem/name") ) {
            FcitxXkbOptionInfo* optionInfo = (FcitxXkbOptionInfo*) utarray_back(optionGroupInfo->optionInfos);
            optionInfo->name = fcitx_utils_strdup(trimmed);
        }
        else if ( fcitx_utils_string_ends_with(strPath, "optionList/group/option/configItem/description") ) {
            FcitxXkbOptionInfo* optionInfo = (FcitxXkbOptionInfo*) utarray_back(optionGroupInfo->optionInfos);
            fcitx_utils_free(optionInfo->description);
            optionInfo->description = fcitx_utils_strdup(trimmed);
        }
        free(strPath);
    }
    free(trimmed);
}


void FcitxXkbLayoutInfoInit(void* arg)
{
    FcitxXkbLayoutInfo* layoutInfo = (FcitxXkbLayoutInfo*) arg;
    memset(layoutInfo, 0, sizeof(FcitxXkbLayoutInfo));
    layoutInfo->languages = fcitx_utils_string_list_new();
    layoutInfo->variantInfos = utarray_new(&variant_icd);
}


void FcitxXkbVariantInfoInit(void* arg)
{
    FcitxXkbVariantInfo* variantInfo = (FcitxXkbVariantInfo*) arg;
    memset(variantInfo, 0, sizeof(FcitxXkbVariantInfo));
    variantInfo->languages = fcitx_utils_string_list_new();
}


void FcitxXkbModelInfoInit(void* arg)
{
    FcitxXkbModelInfo* modelInfo = (FcitxXkbModelInfo*) arg;
    memset(modelInfo, 0, sizeof(FcitxXkbModelInfo));
}


void FcitxXkbOptionGroupInfoInit(void* arg)
{
    FcitxXkbOptionGroupInfo* optionGroupInfo = (FcitxXkbOptionGroupInfo*) arg;
    memset(optionGroupInfo, 0, sizeof(FcitxXkbOptionGroupInfo));
    optionGroupInfo->optionInfos = utarray_new(&option_icd);
}


void FcitxXkbOptionInfoInit(void* arg)
{
    FcitxXkbOptionInfo* optionInfo = (FcitxXkbOptionInfo*) arg;
    memset(optionInfo, 0, sizeof(FcitxXkbOptionInfo));
}


void FcitxXkbLayoutInfoFree(void* arg)
{
    FcitxXkbLayoutInfo* layoutInfo = (FcitxXkbLayoutInfo*) arg;
    fcitx_utils_free(layoutInfo->name);
    fcitx_utils_free(layoutInfo->description);
    utarray_free(layoutInfo->languages);
    utarray_free(layoutInfo->variantInfos);
}


void FcitxXkbVariantInfoFree(void* arg)
{
    FcitxXkbVariantInfo* variantInfo = (FcitxXkbVariantInfo*) arg;
    fcitx_utils_free(variantInfo->name);
    fcitx_utils_free(variantInfo->description);
    utarray_free(variantInfo->languages);
}


void FcitxXkbModelInfoFree(void* arg)
{
    FcitxXkbModelInfo* modelInfo = (FcitxXkbModelInfo*) arg;
    fcitx_utils_free(modelInfo->name);
    fcitx_utils_free(modelInfo->description);
    fcitx_utils_free(modelInfo->vendor);
}


void FcitxXkbOptionGroupInfoFree(void* arg)
{
    FcitxXkbOptionGroupInfo* optionGroupInfo = (FcitxXkbOptionGroupInfo*) arg;
    fcitx_utils_free(optionGroupInfo->name);
    fcitx_utils_free(optionGroupInfo->description);
    utarray_free(optionGroupInfo->optionInfos);
}


void FcitxXkbOptionInfoFree(void* arg)
{
    FcitxXkbOptionInfo* optionInfo = (FcitxXkbOptionInfo*) arg;
    fcitx_utils_free(optionInfo->name);
    fcitx_utils_free(optionInfo->description);
}

void FcitxXkbLayoutInfoCopy(void* dst, const void* src)
{
    FcitxXkbLayoutInfo* layoutInfoDst = (FcitxXkbLayoutInfo*) dst;
    FcitxXkbLayoutInfo* layoutInfoSrc = (FcitxXkbLayoutInfo*) src;
    layoutInfoDst->name = fcitx_utils_strdup(layoutInfoSrc->name);
    layoutInfoDst->description = fcitx_utils_strdup(layoutInfoSrc->description);
    layoutInfoDst->languages = utarray_clone(layoutInfoSrc->languages);
    layoutInfoDst->variantInfos = utarray_clone(layoutInfoSrc->variantInfos);
}

void FcitxXkbModelInfoCopy(void* dst, const void* src)
{
    FcitxXkbModelInfo* modeInfoDst = (FcitxXkbModelInfo*) dst;
    FcitxXkbModelInfo* modeInfoSrc = (FcitxXkbModelInfo*) src;
    modeInfoDst->name = fcitx_utils_strdup(modeInfoSrc->name);
    modeInfoDst->description = fcitx_utils_strdup(modeInfoSrc->description);
    modeInfoDst->vendor = fcitx_utils_strdup(modeInfoSrc->vendor);
}

void FcitxXkbOptionGroupInfoCopy(void* dst, const void* src)
{
    FcitxXkbOptionGroupInfo* optionGroupInfoDst = (FcitxXkbOptionGroupInfo*) dst;
    FcitxXkbOptionGroupInfo* optionGroupInfoSrc = (FcitxXkbOptionGroupInfo*) src;
    optionGroupInfoDst->name = fcitx_utils_strdup(optionGroupInfoSrc->name);
    optionGroupInfoDst->description = fcitx_utils_strdup(optionGroupInfoSrc->description);
    optionGroupInfoDst->exclusive = optionGroupInfoSrc->exclusive;
    optionGroupInfoDst->optionInfos = utarray_clone(optionGroupInfoSrc->optionInfos);
}

void FcitxXkbOptionInfoCopy(void* dst, const void* src)
{
    FcitxXkbOptionInfo* optionInfoDst = (FcitxXkbOptionInfo*) dst;
    FcitxXkbOptionInfo* optionInfoSrc = (FcitxXkbOptionInfo*) src;
    optionInfoDst->name = fcitx_utils_strdup(optionInfoSrc->name);
    optionInfoDst->description = fcitx_utils_strdup(optionInfoSrc->description);
}

void FcitxXkbVariantInfoCopy(void* dst, const void* src)
{
    FcitxXkbVariantInfo* variantInfoDst = (FcitxXkbVariantInfo*) dst;
    FcitxXkbVariantInfo* variantInfoSrc = (FcitxXkbVariantInfo*) src;
    variantInfoDst->name = fcitx_utils_strdup(variantInfoSrc->name);
    variantInfoDst->description = fcitx_utils_strdup(variantInfoSrc->description);
    variantInfoDst->languages = utarray_clone(variantInfoSrc->languages);
}