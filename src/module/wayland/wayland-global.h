/***************************************************************************
 *   Copyright (C) 2013~2013 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
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

#ifndef WAYLAND_GLOBAL_H
#define WAYLAND_GLOBAL_H

#include <stdbool.h>
#include <wayland-util.h>
#include "waylandmodule.h"
#include "fcitx-utils/uthash.h"

typedef struct {
    FcitxWayland *wl;
    struct wl_list obj_list;
} FcitxWaylandIfaceData;

typedef struct {
    FcitxHandlerKey *key;
    struct wl_list link;
    UT_hash_handle hh;
    uint32_t name;
    uint32_t version;
} FcitxWaylandObject;

struct _FcitxWayland
{
    FcitxInstance* instance;
    FcitxInputContextManager* icManager;
    struct wl_display* display;
    FcitxIOEvent* event;
    struct wl_registry* registry;
    FcitxHandlerKey* general_handlers;
    FcitxHandlerTable* global_handlers;
    FcitxWaylandObject *wl_objs;
    struct wl_compositor *compositor;
    struct wl_shell *shell;
    struct wl_shm *shm;
    uint32_t shm_formats;
    struct wl_data_device_manager *data_device_manager;
    struct wl_list input_list;
};

bool FxWaylandRegGlobalHandler(
    FcitxWayland *wl, const char *iface, FcitxWaylandHandleGlobalAdded added,
    FcitxWaylandHandleGlobalRemoved removed, void *data, bool run_exists);
void FxWaylandRemoveGlobalHandler(FcitxWayland *wl, int id);
bool FxWaylandGlobalInit(FcitxWayland *wl);

#endif
