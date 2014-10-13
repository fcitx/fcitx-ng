/***************************************************************************
 *   Copyright (C) 2012~2014 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "config.h"
#include <libintl.h>
#include <xkbcommon/xkbcommon.h>

#include "fcitx/ime.h"

typedef struct _FcitxKeyboard
{
} FcitxKeyboard;

static void* fcitx_keyboard_init(FcitxAddonManager* manager);
static void fcitx_keyboard_destroy(void* data);

FCITX_DEFINE_ADDON(fcitx_keyboard, inputmethod, FcitxAddonAPIInputMethod) = {
    .common = {
        .init = fcitx_keyboard_init,
        .destroy = fcitx_keyboard_destroy
    }
};

void* fcitx_keyboard_init(FcitxAddonManager* manager)
{
    FcitxKeyboard* keyboard = fcitx_utils_new(FcitxKeyboard);
    char* localepath = fcitx_utils_get_fcitx_path("localedir");
    bindtextdomain("xkeyboard-config", localepath);
    bind_textdomain_codeset("xkeyboard-config", "UTF-8");
    free(localepath);

    return keyboard;
}

void fcitx_keyboard_destroy(void* data)
{
    FcitxKeyboard* keyboard = data;
    free(keyboard);
}
