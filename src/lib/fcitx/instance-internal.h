/***************************************************************************
 *   Copyright (C) 2011~2012 by CSSlayer                                   *
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
/**
 * @file   instance-internal.h
 *
 */

#ifndef _FCITX_INSTANCE_INTERNAL_H_
#define _FCITX_INSTANCE_INTERNAL_H_
#include "inputcontext.h"
#include "addon.h"
#include "fcitx-utils/dict.h"

struct _FcitxInstance {
    FcitxDict* inputContexts;
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
};

#endif
