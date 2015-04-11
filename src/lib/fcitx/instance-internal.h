/*
 * Copyright (C) 2011~2015 by CSSlayer
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

/**
 * @file   instance-internal.h
 *
 */

#ifndef _FCITX_INSTANCE_INTERNAL_H_
#define _FCITX_INSTANCE_INTERNAL_H_
#include "inputcontext.h"
#include "addon.h"
#include "ime.h"

struct _FcitxInstance {
    FcitxMainLoop* mainloop;
    char* enableList;
    char* disableList;
    char* uiname;
    FcitxAddonManager* addonManager;
    FcitxStandardPath* standardPath;
    int signalPipe;
    bool tryReplace;
    bool running;
    bool shutdown;
    FcitxInputContextManager* icManager;
    FcitxInputMethodManager* imManager;
    int group;
    int32_t inputMethodStateId;
    FcitxPtrArray* globalInputMethod;
};

#endif
