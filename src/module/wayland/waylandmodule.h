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

#ifndef _FCITX_MODULE_WAYLAND_WAYLANDMODULE_H_
#define _FCITX_MODULE_WAYLAND_WAYLANDMODULE_H_

#include <stdint.h>
#include "fcitx/instance.h"

typedef void (*FcitxWaylandHandleGlobalAdded)(void *data, uint32_t name,
                                              const char *iface, uint32_t ver);
typedef void (*FcitxWaylandHandleGlobalRemoved)(void *data, uint32_t name,
                                                const char *iface,
                                                uint32_t ver);
typedef void (*FcitxWaylandSyncCallback)(void *data, uint32_t serial);

/* need to free buff */
typedef void (*FcitxWaylandReadAllCallback)(void *data, char *buff,
                                            size_t len);


typedef struct _FcitxWayland FcitxWayland;

typedef struct {
    FcitxWaylandHandleGlobalAdded added;
    FcitxWaylandHandleGlobalRemoved removed;
    void *data;
} FcitxWaylandGlobalHandler;

#endif // _FCITX_MODULE_WAYLAND_WAYLANDMODULE_H_
