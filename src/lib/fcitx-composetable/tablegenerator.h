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

#ifndef FCITX_COMPOSETABLE_TABLEGENERATOR_H
#define FCITX_COMPOSETABLE_TABLEGENERATOR_H

#define FCITX_KEYSEQUENCE_MAX_LEN 7

typedef struct _FcitxComposeTable FcitxComposeTable;

typedef enum _FcitxComposeTableState {
    FCTS_NoErrors = 0, 
    FCTS_UnsupportedLocale,
    FCTS_EmptyTable,
    FCTS_UnknownSystemComposeDir,
    FCTS_MissingComposeFile,
} FcitxComposeTableState;

FcitxComposeTable* fcitx_compose_table_new_from_file(const char* systemComposeDir, const char* composeFile, const char* locale);
FcitxComposeTable* fcitx_compose_table_new(const char* locale);
FcitxComposeTable* fcitx_compose_table_ref(FcitxComposeTable* table);
void fcitx_compose_table_unref(FcitxComposeTable* table);
void fcitx_compose_table_print(FcitxComposeTable* table);
FcitxComposeTableState fcitx_compose_table_state(FcitxComposeTable* table);

#endif // FCITX_COMPOSETABLE_TABLEGENERATOR_H
