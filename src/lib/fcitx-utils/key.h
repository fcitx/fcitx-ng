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

#ifndef _FCITX_UTILS_KEY_H_
#define _FCITX_UTILS_KEY_H_

#include "macro.h"
#include "keysym.h"
#include "types.h"

FCITX_DECL_BEGIN

typedef uint32_t FcitxKeyStates;

typedef struct _FcitxKey
{
    FcitxKeySym sym; /** keyval of hotkey */
    FcitxKeyStates state; /** state of hotkey */
} FcitxKey;

typedef struct _FcitxKeyList FcitxKeyList;

#define FCITX_KEY(SYM, STATE) ((FcitxKey) {SYM, STATE})


FcitxKey fcitx_key_parse(const char* keyString);
char* fcitx_key_to_string(FcitxKey hotkey);
bool fcitx_key_check(FcitxKey toCheck, FcitxKey key);

bool fcitx_key_is_digit(FcitxKey key);
bool fcitx_key_is_uaz(FcitxKey key);
bool fcitx_key_is_laz(FcitxKey key);
bool fcitx_key_is_simple(FcitxKey key);
bool fcitx_key_is_modifier(FcitxKey key);
bool fcitx_key_is_cursor_move(FcitxKey key);

FcitxKey fcitx_key_normalize(FcitxKey key);

FcitxKeyList* fcitx_key_list_new(void);
FcitxKeyList* fcitx_key_list_new_from_string(const char* keyString);
void fcitx_key_list_parse(FcitxKeyList* keyList, const char* keyString);
void fcitx_key_list_free(FcitxKeyList* keyList);
bool fcitx_key_list_check(FcitxKeyList* keyList, FcitxKey key);
void fcitx_key_list_append(FcitxKeyList* keyList, FcitxKey key);
void fcitx_key_list_clear(FcitxKeyList* keyList);
char* fcitx_key_list_to_string(FcitxKeyList* keyList);


const char* fcitx_keysym_to_string (FcitxKeySym keysym);
FcitxKeySym fcitx_keysym_from_string(const char* str);

/**
 * convert unicode character to keyval
 *
 * If No matching keysym value found, return Unicode value plus 0x01000000
 * (a convention introduced in the UTF-8 work on xterm).
 *
 * @param wc unicode
 * @return FcitxKeySym
 **/
FcitxKeySym fcitx_keysym_from_unicode (uint32_t wc);

/**
 * convert keyval to unicode character
 *
 * @param keyval keyval
 * @return unicode
 **/
uint32_t fcitx_keysym_to_unicode (FcitxKeySym keyval);

FcitxKeyStates fcitx_keysym_to_state(FcitxKeySym sym);

FCITX_DECL_END

#endif //  _FCITX_UTILS_KEY_H_
