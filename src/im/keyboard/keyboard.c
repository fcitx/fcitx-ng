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
#include "fcitx-xcb.h"

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
static bool fcitx_keyboard_handle_event(void* data, FcitxEvent* event);

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

    keyboard->manager = manager;
    keyboard->standardPath = fcitx_standard_path_ref(fcitx_addon_manager_get_standard_path(manager));
    keyboard->immanager = fcitx_input_method_manager_ref(fcitx_addon_manager_get_property(manager, "immanager"));

    return keyboard;
}

void fcitx_keyboard_destroy(void* data)
{
    FcitxKeyboard* keyboard = data;
    free(keyboard);
}

bool fcitx_keyboard_handle_event(void* data, FcitxEvent* event)
{
    switch (event->type) {
        default:
            break;
    }
    return false;
}
