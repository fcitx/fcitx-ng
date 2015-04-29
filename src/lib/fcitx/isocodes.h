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

#ifndef FCITX_ISOCODES_H
#define FCITX_ISOCODES_H

#include "fcitx-utils/uthash.h"
#include "fcitx-utils/utils.h"

typedef struct _FcitxIsoCodes639Entry {
    char* name;
    char* iso_639_2B_code;
    char* iso_639_2T_code;
    char* iso_639_1_code;
    UT_hash_handle hh1;
    UT_hash_handle hh2;
} FcitxIsoCodes639Entry;

typedef struct _FcitxIsoCodes3166Entry {
    char* name;
    char* alpha_2_code;
    UT_hash_handle hh;
} FcitxIsoCodes3166Entry;

typedef struct _FcitxIsoCodes {
    FcitxIsoCodes639Entry* iso6392B;
    FcitxIsoCodes639Entry* iso6392T;
    FcitxIsoCodes3166Entry* iso3166;
} FcitxIsoCodes;

FcitxIsoCodes* fcitx_isocodes_new(const char* iso639, const char* iso3166);
FcitxIsoCodes639Entry* fcitx_isocodes_get(FcitxIsoCodes* isocodes, const char* lang);
const char* fcitx_isocodes_find_best(FcitxIsoCodes* isocodes, const char* hint, UT_array* languages);
void fcitx_isocodes_free(FcitxIsoCodes* isocodes);

#endif
