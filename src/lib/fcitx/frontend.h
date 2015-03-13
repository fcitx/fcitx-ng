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

#ifndef __FCITX_FRONTEND_H__
#define __FCITX_FRONTEND_H__

#include "addon.h"
#include "inputcontext.h"

typedef struct _FcitxAddonAPIFrontend
{
    FcitxAddonAPICommon common;
    void (*createIC)(void* arg, FcitxInputContext*, void* priv); /**< frontend create input context callback */
    void (*destroyIC)(void* arg, FcitxInputContext *context); /**< frontend destroy input context callback */
    void (*commitString)(void* arg, FcitxInputContext* arg1, const char* arg2); /**< frontend commit string callback */
    void (*forwardKey)(void* arg, FcitxInputContext* arg1, FcitxKeyEvent event); /**< frontend forward key callback */
    void (*getWindowRect)(void* arg, FcitxInputContext* ic, int* x, int* y, int* w, int* h); /**< frontend get window position callback */
    void (*updatePreedit)(void* arg, FcitxInputContext* ic); /**< frontend update preedit callback */
    void (*updateClientSideUI)(void* arg, FcitxInputContext* ic); /**< frontend update client side user interface callback */
    void (*deleteSurroundingText)(void* addonInstance, FcitxInputContext* ic, int offset, unsigned int size);
    bool (*getSurroundingText)(void* addonInstance, FcitxInputContext* ic, char** str, unsigned int* cursor, unsigned int* anchor);
    void* padding1;
    void* padding2;
    void* padding3;
    void* padding4;
    void* padding5;
    void* padding6;
    void* padding7;
    void* padding8;
} FcitxAddonAPIFrontend;

#endif // __FCITX_FRONTEND_H__
