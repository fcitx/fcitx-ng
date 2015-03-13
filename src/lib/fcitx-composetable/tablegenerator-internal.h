/*
 * Copyright (C) 2014~2015 by CSSlayer
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

#ifndef FCITX_COMPOSE_TABLE_TABLEGENERATOR_INTERNAL_H
#define FCITX_COMPOSE_TABLE_TABLEGENERATOR_INTERNAL_H

#include "tablegenerator.h"
#include "fcitx-utils/utils.h"

typedef struct _FcitxComposeTableElement {
    FcitxKeySym keys[FCITX_KEYSEQUENCE_MAX_LEN];
    uint32_t value;
} FcitxComposeTableElement;

struct _FcitxComposeTable
{
    int32_t refcount;
    char* systemComposeDir;
    FcitxDict* localeToTable;
    FcitxComposeTableState state;
    UT_array* composeTable;
    char* locale;
};

int fcitx_compose_table_element_cmp(const void* a, const void* b, void* data);

#endif // FCITX_COMPOSE_TABLE_TABLEGENERATOR_INTERNAL_H
