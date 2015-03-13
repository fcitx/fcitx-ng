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

typedef enum _FcitxIMCloseEventType {
    /**
     * when user press inactivate key, default behavior is commit raw preedit.
     * If you want to OVERRIDE this behavior, be sure to implement this function.
     *
     * in some case, your implementation of OnClose should respect the value of
     * [Output/SendTextWhenSwitchEng], when this value is true, commit something you
     * want.
     *
     * And no matter in which case, Reset will be called after that.
     *
     * CET_ChangeByUser will not be emitted once CET_ChangeByInactivate is emitted.
     */
    CET_ChangeByInactivate,
    /**
     * when using lost focus
     * this might be variance case to case. the default behavior is to commit
     * the preedit, and resetIM.
     *
     * Controlled by [Output/DontCommitPreeditWhenUnfocus], this option will not
     * work for application switch doesn't support async commit.
     *
     * So OnClose is called when preedit IS committed (not like CET_ChangeByInactivate,
     * this behavior cannot be overrided), it give im a chance to choose remember this
     * word or not.
     *
     * Input method need to notice, that the commit is already DONE, do not do extra commit.
     */
    CET_LostFocus,
    /**
     * when user switch to a different input method by hand
     * such as ctrl+shift by default, or by ui,
     * default behavior is reset IM.
     */
    CET_SwitchIM,
    CET_ChangeByUser = CET_SwitchIM, // the old name is not accurate, but keep for compatible.
} FcitxIMCloseEventType;

typedef enum _FcitxIMEventType
{
    IET_Init,
    IET_Keyboard,
    IET_Reset,
    IET_SurroundingTextUpdated,
} FcitxIMEventType;

typedef struct _FcitxIMEvent {
    FcitxIMEventType type;
} FcitxIMEvent;

typedef FcitxIMEvent FcitxIMInitEvent;
typedef FcitxIMEvent FcitxIMSurroundingTextUpdatedEvent;

typedef struct _FcitxIMKeyboardEvent {
    FcitxIMEventType type;
    FcitxKeyEvent key;
} FcitxIMKeyboardEvent;

typedef struct _FcitxIMResetEvent {
    FcitxIMEventType type;
    FcitxIMCloseEventType detail;
} FcitxIMResetEvent;

typedef struct _FcitxAddonAPIInputMethod
{
    FcitxAddonAPICommon common;
} FcitxAddonAPIInputMethod;

typedef struct _FcitxInputMethodManager FcitxInputMethodManager;

FcitxInputMethodManager* fcitx_input_method_manager_new();
FcitxInputMethodManager* fcitx_input_method_manager_ref(FcitxInputMethodManager* manager);
void fcitx_input_method_manager_unref(FcitxInputMethodManager* manager);

bool fcitx_input_method_manager_register(FcitxInputMethodManager* manager,
                                         void *imclass,
                                         const char* uniqueName,
                                         const char* name,
                                         const char* iconName,
                                         bool (*handleEvent)(void* arg, const FcitxIMEvent* event),
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

void fcitx_input_method_manager_set_input_method_list(FcitxInputMethodManager* manager, int group, const char* const * ims);


#endif // _FCITX_IME_H_
