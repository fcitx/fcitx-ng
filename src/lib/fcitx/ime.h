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
#ifndef _FCITX_IME_H_
#define _FCITX_IME_H_

#include "addon.h"
#include "inputcontext.h"

typedef struct _FcitxAddonAPIInputMethod
{
    FcitxAddonAPICommon common;
} FcitxAddonAPIInputMethod;

typedef enum _FcitxInputMethodItemProperty
{
    FIMIP_Name = 0x1,
    FIMIP_Layout = 0x2,
    FIMIP_Variant = 0x3,

    FIMIP_None = 0x0
} FcitxInputMethodItemProperty;

typedef struct _FcitxInputMethodItem FcitxInputMethodItem;

typedef struct _FcitxInputMethodManager FcitxInputMethodManager;

FcitxInputMethodManager* fcitx_input_method_manager_new();
FcitxInputMethodManager* fcitx_input_method_manager_ref(FcitxInputMethodManager* manager);
void fcitx_input_method_manager_unref(FcitxInputMethodManager* manager);

bool fcitx_input_method_manager_register(FcitxInputMethodManager* manager,
                                         void *imclass,
                                         const char* uniqueName,
                                         const char* name,
                                         const char* iconName,
                                         FcitxDispatchEventCallback handleEvent,
                                         int priority,
                                         const char *langCode);

/**
 * now we have a concept of input method group
 * this is to handle the global layout change and local layout change
 * first of all, there's a default group, which will be used by all input device.
 *
 * For each group, there's a default layout for all input method, every input method
 * can be registered multiple times.
 *
 * Use case:
 * Kain has two keyboards. One is laptop built-in with US layout, which he will use at home. Another
 * is a usb keyboard, but it's in fr layout.
 *
 * He usually use Pinyin and Mozc to type Chinese and Japanese.
 * His configuration will look like this:
 * Default group (us):
 * Keyboard (default), Pinyin (default), Mozc (Japanese)
 *
 * Alternative group (fr):
 * Use the same as default.
 *
 * Once fcitx supports hardware detection, it will be able to automatically switch between different
 * groups.
 */
int fcitx_input_method_manager_create_group(FcitxInputMethodManager* manager, ...);

void fcitx_input_method_manager_reset_group(FcitxInputMethodManager* manager);

size_t fcitx_input_method_manager_group_count(FcitxInputMethodManager* manager);

void fcitx_input_method_manager_set_input_method_list(FcitxInputMethodManager* manager, int groupId, const char* const * ims);

bool fcitx_input_method_manager_is_group_empty(FcitxInputMethodManager* manager, int groupId);

FcitxInputMethodItem* fcitx_input_method_manager_get_group_item(FcitxInputMethodManager* manager, int groupId, size_t index);

size_t fcitx_input_method_manager_get_group_size(FcitxInputMethodManager* manager, int groupId);

void fcitx_input_method_manager_set_event_dispatcher(FcitxInputMethodManager* manager, FcitxDispatchEventCallback callback, FcitxDestroyNotify destroyNotify, void* userData);

void fcitx_input_method_item_get_property(FcitxInputMethodItem* item, ...);

#endif // _FCITX_IME_H_
