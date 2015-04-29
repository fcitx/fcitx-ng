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

#ifndef FCITX_KEYBOARD_RULES_H
#define FCITX_KEYBOARD_RULES_H

#include "fcitx-utils/utils.h"

typedef struct _FcitxXkbRules {
    UT_array* layoutInfos;
    UT_array* modelInfos;
    UT_array* optionGroupInfos;
    char* version;
} FcitxXkbRules;

typedef struct _FcitxXkbRulesHandler {
    UT_array* path;
    FcitxXkbRules* rules;
    bool fromExtra;
} FcitxXkbRulesHandler;

typedef struct _FcitxXkbLayoutInfo {
    UT_array* variantInfos;
    char* name;
    char* description;
    UT_array* languages;
} FcitxXkbLayoutInfo;

typedef struct _FcitxXkbVariantInfo {
    char* name;
    char* description;
    UT_array* languages;
} FcitxXkbVariantInfo;

typedef struct _FcitxXkbModelInfo {
    char* name;
    char* description;
    char* vendor;
} FcitxXkbModelInfo;

typedef struct _FcitxXkbOptionGroupInfo {
    UT_array* optionInfos;
    char* name;
    char* description;
    bool exclusive;
} FcitxXkbOptionGroupInfo;

typedef struct _FcitxXkbOptionInfo {
    char* name;
    char* description;
} FcitxXkbOptionInfo;

FcitxXkbRules* fcitx_xkb_rules_new(const char* file);
void fcitx_xkb_rules_free(FcitxXkbRules* rules);

#endif
