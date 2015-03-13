/*
 * Copyright (C) 2014~2015 by CSSlayer
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

#ifndef _FCITX_UTILS_MAINLOOP_H_
#define _FCITX_UTILS_MAINLOOP_H_

#include "types.h"

typedef struct _FcitxMainLoop FcitxMainLoop;
typedef struct _FcitxIOEvent FcitxIOEvent;
typedef struct _FcitxTimeoutEvent FcitxTimeoutEvent;

typedef void (*FcitxIOEventCallback)(FcitxIOEvent* event, int fd, unsigned int flag, void* data);
typedef void (*FcitxTimeoutEventCallback)(FcitxTimeoutEvent* event, void* data);

typedef enum _FcitxIOEventFlag
{
    FIOEF_IN = (1 << 0),
    FIOEF_OUT = (1 << 1),
    FIOEF_ERR = (1 << 2),
    FIOEF_HUP = (1 << 3),
} FcitxIOEventFlag;

FcitxMainLoop* fcitx_mainloop_new(void);
int fcitx_mainloop_run(FcitxMainLoop* mainloop);
void fcitx_mainloop_quit(FcitxMainLoop* mainloop);
FcitxIOEvent* fcitx_mainloop_register_io_event(FcitxMainLoop* mainloop, int fd, unsigned int flag, FcitxIOEventCallback callback, FcitxDestroyNotify freeFunc, void* userdata);
FcitxTimeoutEvent* fcitx_mainloop_register_timeout_event(FcitxMainLoop* mainloop, uint32_t timeout, bool repeat, FcitxTimeoutEventCallback callback, FcitxDestroyNotify freeFunc, void* userdata);
void fcitx_mainloop_remove_io_event(FcitxMainLoop* mainloop, FcitxIOEvent* event);
void fcitx_mainloop_remove_timeout_event(FcitxMainLoop* mainloop, FcitxTimeoutEvent* event);
void fcitx_mainloop_free(FcitxMainLoop* mainloop);

#endif
