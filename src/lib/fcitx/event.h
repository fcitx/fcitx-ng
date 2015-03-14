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

#ifndef _FCITX_EVENT_H_
#define _FCITX_EVENT_H_


typedef enum _FcitxIMResetEventReason {
    CET_ChangeByInactivate,
    CET_LostFocus,
    CET_SwitchIM,
} FcitxIMResetEventReason;

typedef enum _FcitxEventType
{
    ET_EventTypeFlag = 0xffff0000,
    ET_UserTypeFlag = 0xffff0000,
    ET_InputContextEventFlag = 0x0001000,
    ET_InputMethodEventFlag = 0x0002000,
    ET_InstanceEventFlag = 0x0003000,
    // send by frontend, captured by core, input method, or module
    ET_InputContextCreated = ET_InputContextEventFlag | 0x1,
    ET_InputContextDestroyed = ET_InputContextEventFlag | 0x2,
    ET_InputContextFocusIn = ET_InputContextEventFlag | 0x3,
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
    ET_InputContextFocusOut = ET_InputContextEventFlag | 0x4,
    ET_InputContextKeyEvent = ET_InputContextEventFlag | 0x5,
    ET_InputContextReset = ET_InputContextEventFlag | 0x6,
    ET_InputContextSurroundingTextUpdated = ET_InputContextEventFlag | 0x7,
    ET_InputContextCapabilityChanged = ET_InputContextEventFlag | 0x8,
    ET_InputContextCursorRectChanged = ET_InputContextEventFlag | 0x9,
    /**
     * when user switch to a different input method by hand
     * such as ctrl+shift by default, or by ui,
     * default behavior is reset IM.
     */
    ET_InputContextSwitchInputMethod = ET_InputContextEventFlag | 0xA,
    /**
     * when user press inactivate key, default behavior is commit raw preedit.
     * If you want to OVERRIDE this behavior, be sure to implement this function.
     *
     * in some case, your implementation of OnClose should respect the value of
     * [Output/SendTextWhenSwitchEng], when this value is true, commit something you
     * want.
     */
    ET_InputContextInactivate = ET_InputContextEventFlag | 0xB,

    // send by im, captured by frontend, or module
    ET_InputContextForwardKey = ET_InputMethodEventFlag | 0x1,
    ET_InputContextCommitString = ET_InputMethodEventFlag | 0x2,
    ET_InputContextDeleteSurroundingText = ET_InputMethodEventFlag | 0x3,
    ET_InputContextUpdatePreedit = ET_InputMethodEventFlag | 0x4,

    /**
     * captured by everything
     * This would also trigger ET_InputContextSwitchInputMethod afterwards.
     */
    ET_InputMethodGroupChanged = ET_InstanceEventFlag | 0x1,
} FcitxEventType;

typedef struct _FcitxEvent {
    FcitxEventType type;
} FcitxEvent;

typedef bool (*FcitxDispatchEventCallback)(void* data, FcitxEvent* event);

#endif // _FCITX_EVENT_H_
