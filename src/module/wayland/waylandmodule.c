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

#include <wayland-client.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "wayland-global.h"
#include "wayland-input.h"
#include "fcitx-wayland-internal.h"

typedef struct {
    int id;
    const char *iface_name;
    const struct wl_interface *iface;
    void (**listener)();
    FcitxWayland *wl;
    struct wl_proxy **ret;
    void (*destroy)(struct wl_proxy *proxy);
    bool failed;
    uint32_t ver;
} FxWaylandSingletonListener;

#define FXWL_DEF_SINGLETON(_wl, field, name, _iface, _listener,         \
                           _destroy, _ver) {                            \
        .id = -1,                                                       \
        .iface_name = name,                                             \
        .iface = &_iface,                                               \
        .wl = _wl,                                                      \
        .ret = (struct wl_proxy**)(&_wl->field),                         \
        .listener = (void (**)())_listener,                             \
        .destroy = (void (*)(struct wl_proxy*))_destroy,                \
        .failed = true,                                                 \
        .ver = _ver                                                     \
    }

static void
FxWaylandHandleSingletonAdded(void *data, uint32_t name, const char *iface,
                              uint32_t ver)
{
    FxWaylandSingletonListener *singleton = data;
    FcitxWayland *wl = singleton->wl;
    if (*singleton->ret) {
        singleton->failed = true;
        return;
    }
    struct wl_proxy *proxy =
        wl_registry_bind(wl->registry, name, singleton->iface, singleton->ver);
    *singleton->ret = proxy;
    singleton->failed = false;
    if (singleton->listener) {
        wl_proxy_add_listener(proxy, singleton->listener, wl);
    }
}

static void* fcitx_wayland_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_wayland_destroy(void* data);

FCITX_DEFINE_ADDON(fcitx_wayland, module, FcitxAddonAPICommon) = {
    .init = fcitx_wayland_init,
    .destroy = fcitx_wayland_destroy,
    .registerCallback = fcitx_wayland_register_functions
};

static void
FxWaylandShmFormatHandler(void *data, struct wl_shm *shm, uint32_t format)
{
    FcitxWayland *wl = data;
    FCITXGCLIENT_UNUSED(shm);
    wl->shm_formats |= (1 << format);
}

static const struct wl_shm_listener fx_shm_listenr = {
    .format = FxWaylandShmFormatHandler
};

void fcitx_wayland_io_callback(FcitxIOEvent* _event, int fd, unsigned int flag, void* data)
{
    FCITXGCLIENT_UNUSED(_event);
    FCITXGCLIENT_UNUSED(fd);
    FCITXGCLIENT_UNUSED(flag);

    FcitxWayland* wayland = data;
    if (wl_display_dispatch(wayland->display)) {
    }
}

void* fcitx_wayland_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITXGCLIENT_UNUSED(config);
    FcitxWayland* wayland = fcitx_utils_new(FcitxWayland);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    FcitxInputContextManager* icManager = fcitx_addon_manager_get_property(manager, "icmanager");
    wayland->instance = instance;
    wayland->icManager = icManager;

    wayland->display = wl_display_connect(NULL);
    if (!wayland->display) {
        goto wayland_failed;
    }

    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(wayland->instance);
    wayland->event = fcitx_mainloop_register_io_event(mainloop, wl_display_get_fd(wayland->display), FIOEF_IN, fcitx_wayland_io_callback, NULL, wayland);

    wayland->registry = wl_display_get_registry(wayland->display);
    if (fcitx_unlikely(!wayland->registry))
        goto wayland_remove_io_event;
    if (fcitx_unlikely(!FxWaylandGlobalInit(wayland)))
        goto destroy_registry;
    FxWaylandSingletonListener singleton_listeners[] = {
        FXWL_DEF_SINGLETON(wayland, compositor, "wl_compositor",
                           wl_compositor_interface, NULL,
                           wl_compositor_destroy, 2),
        FXWL_DEF_SINGLETON(wayland, shell, "wl_shell", wl_shell_interface, NULL,
                           wl_shell_destroy, 1),
        FXWL_DEF_SINGLETON(wayland, shm, "wl_shm", wl_shm_interface,
                           &fx_shm_listenr, wl_shm_destroy, 1),
        FXWL_DEF_SINGLETON(wayland, data_device_manager, "wl_data_device_manager",
                           wl_data_device_manager_interface, NULL,
                           wl_data_device_manager_destroy, 1),
    };

    const int singleton_count =
        sizeof(singleton_listeners) / sizeof(singleton_listeners[0]);
    int i;
    for (i = 0;i < singleton_count;i++) {
        FxWaylandSingletonListener *listener = singleton_listeners + i;
        listener->id = FxWaylandRegGlobalHandler(wayland, listener->iface_name,
                                                 FxWaylandHandleSingletonAdded,
                                                 NULL, listener, true);
    }
    wl_display_roundtrip(wayland->display);
    bool failed = false;
    for (i = 0;i < singleton_count;i++) {
        FxWaylandSingletonListener *listener = singleton_listeners + i;
        FxWaylandRemoveGlobalHandler(wayland, listener->id);
        if (listener->failed) {
            failed = true;
        }
    }
    if (failed)
        goto free_handlers;
    FxWaylandInputInit(wayland);
    return wayland;
free_handlers:
    for (i = 0;i < singleton_count;i++) {
        FxWaylandSingletonListener *listener = singleton_listeners + i;
        if (*listener->ret) {
            listener->destroy(*listener->ret);
        }
    }
    fcitx_handler_table_free(wayland->global_handlers);
destroy_registry:
    wl_registry_destroy(wayland->registry);

    return wayland;
wayland_remove_io_event:
    fcitx_mainloop_remove_io_event(mainloop, wayland->event);
wayland_disconnect:
    wl_display_disconnect(wayland->display);
wayland_failed:
    free(wayland);
    return NULL;
}

void fcitx_wayland_destroy(void* data)
{
    FcitxWayland* wayland = data;
    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(wayland->instance);
    fcitx_mainloop_remove_io_event(mainloop, wayland->event);
    wl_display_disconnect(wayland->display);
    free(wayland);
}

struct wl_display* fcitx_wayland_get_display(FcitxWayland* self)
{
    return self->display;
}
