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

#include <dbus/dbus.h>
#include <unistd.h>
#include <assert.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "dbusmodule.h"
#include "fcitx-dbus-internal.h"

#define FCITX_CONTROLLER_DBUS_INTERFACE "org.fcitx.Fcitx.Controller"

const char * controller_introspection_xml =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
    "<node>"
    "<interface name=\"" DBUS_INTERFACE_INTROSPECTABLE "\">"
    "<method name=\"Introspect\">"
    "<arg name=\"data\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "<interface name=\"" DBUS_INTERFACE_PROPERTIES "\">"
    "<method name=\"Get\">"
    "<arg name=\"interface_name\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"property_name\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"value\" direction=\"out\" type=\"v\"/>"
    "</method>"
    "<method name=\"Set\">"
    "<arg name=\"interface_name\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"property_name\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"value\" direction=\"in\" type=\"v\"/>"
    "</method>"
    "<method name=\"GetAll\">"
    "<arg name=\"interface_name\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"values\" direction=\"out\" type=\"a{sv}\"/>"
    "</method>"
    "<signal name=\"PropertiesChanged\">"
    "<arg name=\"interface_name\" type=\"s\"/>"
    "<arg name=\"changed_properties\" type=\"a{sv}\"/>"
    "<arg name=\"invalidated_properties\" type=\"as\"/>"
    "</signal>"
    "</interface>"
    "<interface name=\"" FCITX_CONTROLLER_DBUS_INTERFACE "\">"
    "<method name=\"Exit\">"
    "</method>"
    "<method name=\"GetCurrentIM\">"
    "<arg name=\"im\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "<method name=\"SetCurrentIM\">"
    "<arg name=\"im\" direction=\"in\" type=\"s\"/>"
    "</method>"
    "<method name=\"ReloadConfig\">"
    "</method>"
    "<method name=\"ReloadAddonConfig\">"
    "<arg name=\"addon\" direction=\"in\" type=\"s\"/>"
    "</method>"
    "<method name=\"Restart\">"
    "</method>"
    "<method name=\"Configure\">"
    "</method>"
    "<method name=\"ConfigureAddon\">"
    "<arg name=\"addon\" direction=\"in\" type=\"s\"/>"
    "</method>"
    "<method name=\"ConfigureIM\">"
    "<arg name=\"im\" direction=\"in\" type=\"s\"/>"
    "</method>"
    "<method name=\"GetCurrentUI\">"
    "<arg name=\"addon\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "<method name=\"GetIMAddon\">"
    "<arg name=\"im\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"addon\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "<method name=\"ActivateIM\">"
    "</method>"
    "<method name=\"InactivateIM\">"
    "</method>"
    "<method name=\"ToggleIM\">"
    "</method>"
    "<method name=\"ResetIMList\">"
    "</method>"
    "<method name=\"GetCurrentState\">"
    "<arg name=\"state\" direction=\"out\" type=\"i\"/>"
    "</method>"
    "<property access=\"readwrite\" type=\"a(sssb)\" name=\"IMList\">"
    "<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\"/>"
    "</property>"
    "<property access=\"readwrite\" type=\"s\" name=\"CurrentIM\">"
    "<annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\"/>"
    "</property>"
    "</interface>"
    "</node>";

struct _FcitxDBus
{
    FcitxInstance* instance;
    FcitxMainLoop* mainloop;
    DBusConnection* conn;
    bool shutdown;
    FcitxTimeoutEvent* wakeup_main;
    bool objectRegistered;
};

typedef struct _FcitxDBusWakeUpMainData
{
    FcitxDBus* dbus;
    DBusConnection* conn;
} FcitxDBusWakeUpMainData;

static const int RETRY_INTERVAL = 1;
static const int MAX_RETRY_TIMES = 5;

static void* fcitx_dbus_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_dbus_destroy(void* data);
static DBusHandlerResult fcitx_dbus_filter(DBusConnection* connection, DBusMessage* msg, void* user_data);

FCITX_DEFINE_ADDON(fcitx_dbus, module, FcitxAddonAPICommon) = {
    .init = fcitx_dbus_init,
    .destroy = fcitx_dbus_destroy,
    .registerCallback = fcitx_dbus_register_functions
};


void fcitx_dbus_watch_callback(FcitxIOEvent* _event, int fd, unsigned int flag, void* data)
{
    FCITX_UNUSED(_event);
    FCITX_UNUSED(fd);

    unsigned int dflag =
           ((flag & FIOEF_IN) ? DBUS_WATCH_READABLE : 0)
         | ((flag & FIOEF_OUT) ? DBUS_WATCH_WRITABLE : 0)
         | ((flag & FIOEF_ERR) ? DBUS_WATCH_ERROR : 0)
         | ((flag & FIOEF_HUP) ? DBUS_WATCH_HANGUP : 0);
    DBusWatch* watch = data;
    assert(fd == dbus_watch_get_unix_fd(watch));
    dbus_watch_handle(watch, dflag);
}

void fcitx_dbus_watch_remove_notify(void* data)
{
    DBusWatch* watch = data;
    dbus_watch_set_data(watch, NULL, NULL);
}

dbus_bool_t fcitx_dbus_add_watch(DBusWatch *watch, void *data)
{
    FcitxDBus* dbus = data;
    if (!dbus_watch_get_enabled(watch)) {
        return TRUE;
    }

    unsigned int flags = dbus_watch_get_flags (watch);
    int fflag = 0;
    int fd = dbus_watch_get_unix_fd(watch);

    if (flags & DBUS_WATCH_READABLE)
        fflag |= FIOEF_IN;
    if (flags & DBUS_WATCH_WRITABLE)
        fflag |= FIOEF_OUT;

    fflag |= FIOEF_ERR | FIOEF_HUP;

    FcitxIOEvent* event = fcitx_mainloop_register_io_event(dbus->mainloop, fd, fflag, fcitx_dbus_watch_callback, fcitx_dbus_watch_remove_notify, watch);
    if (event) {
        dbus_watch_set_data(watch, event, NULL);
        return TRUE;
    }

    return FALSE;
}

void fcitx_dbus_remove_watch(DBusWatch *watch, void *data)
{
    FcitxDBus* dbus = data;
    FcitxIOEvent* event = dbus_watch_get_data(watch);
    if (event) {
        fcitx_mainloop_remove_io_event(dbus->mainloop, event);
    }
}

void fcitx_dbus_toggle_watch(DBusWatch *watch, void *data)
{
    if (dbus_watch_get_enabled (watch))
        fcitx_dbus_add_watch (watch, data);
    else
        fcitx_dbus_remove_watch (watch, data);
}

void fcitx_dbus_timeout_remove_notify(void* data)
{
    DBusTimeout* timeout = data;
    dbus_timeout_set_data(timeout, NULL, NULL);
}

void fcitx_dbus_timeout_callback(FcitxTimeoutEvent* _event, void* data)
{
    FCITX_UNUSED(_event);
    DBusTimeout* timeout = data;
    dbus_timeout_handle(timeout);
}

dbus_bool_t fcitx_dbus_add_timeout(DBusTimeout *timeout, void *data)
{
    FcitxDBus* dbus = data;
    if (!dbus_timeout_get_enabled(timeout)) {
        return TRUE;
    }

    int interval = dbus_timeout_get_interval (timeout);

    FcitxTimeoutEvent* event = fcitx_mainloop_register_timeout_event(dbus->mainloop, interval, true, fcitx_dbus_timeout_callback, NULL, timeout);
    if (event) {
        dbus_timeout_set_data(timeout, event, NULL);
        return TRUE;
    }

    return FALSE;
}

void fcitx_dbus_remove_timeout(DBusTimeout *timeout, void *data)
{
    FcitxDBus* dbus = data;
    FcitxTimeoutEvent* event = dbus_timeout_get_data(timeout);
    if (event) {
        fcitx_mainloop_remove_timeout_event(dbus->mainloop, event);
    }
}

void fcitx_dbus_toggle_timeout(DBusTimeout *timeout, void *data)
{
    if (dbus_timeout_get_enabled (timeout))
        fcitx_dbus_add_timeout (timeout, data);
    else
        fcitx_dbus_remove_timeout (timeout, data);
}

void fcitx_dbus_dispatch(FcitxTimeoutEvent* event, void* _data)
{
    FCITX_UNUSED(event);
    FcitxDBusWakeUpMainData* data = _data;
    data->dbus->wakeup_main = NULL;
    dbus_connection_ref(data->conn);
    while (dbus_connection_dispatch(data->conn) == DBUS_DISPATCH_DATA_REMAINS);
    dbus_connection_unref(data->conn);
}

void fcitx_dbus_wakeup_main(void* _data)
{
    FcitxDBusWakeUpMainData* data = _data;
    if (data->dbus->shutdown) {
        return;
    }
    data->dbus->wakeup_main = fcitx_mainloop_register_timeout_event(data->dbus->mainloop, 0, false, fcitx_dbus_dispatch, NULL, data);
}

bool fcitx_dbus_setup_connection(FcitxDBus* dbus, DBusConnection* conn)
{
    if (!dbus_connection_add_filter(conn, fcitx_dbus_filter, dbus, NULL)) {
        return false;
    }

    if (!dbus_connection_set_watch_functions(conn,
                                             fcitx_dbus_add_watch,
                                             fcitx_dbus_remove_watch,
                                             fcitx_dbus_toggle_watch,
                                             dbus, NULL)) {
        return false;
    }

    if (!dbus_connection_set_timeout_functions(conn,
                                               fcitx_dbus_add_timeout,
                                               fcitx_dbus_remove_timeout,
                                               fcitx_dbus_toggle_timeout,
                                               dbus, NULL)) {
        return false;
    }


    FcitxDBusWakeUpMainData* data = fcitx_utils_new(FcitxDBusWakeUpMainData);
    if (!data) {
        return false;
    }
    data->conn = conn;
    data->dbus = dbus;
    dbus_connection_set_wakeup_main_function(conn, fcitx_dbus_wakeup_main, data, free);

    return true;
}

static DBusHandlerResult fcitx_dbus_controller_handler(DBusConnection *connection, DBusMessage *msg, void *user_data)
{
    FcitxDBus* dbus = user_data;
    DBusMessage* reply = NULL;
    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        reply = dbus_message_new_method_return(msg);
        dbus_message_append_args(reply, DBUS_TYPE_STRING, &controller_introspection_xml, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, FCITX_CONTROLLER_DBUS_INTERFACE, "Exit")) {
        reply = dbus_message_new_method_return(msg);
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        dbus_connection_flush(connection);
        fcitx_instance_shutdown(dbus->instance);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (reply) {
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        dbus_connection_flush(connection);
        result = DBUS_HANDLER_RESULT_HANDLED;
    }

    return result;
}

void* fcitx_dbus_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxDBus* dbus = fcitx_utils_new(FcitxDBus);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    dbus->instance = instance;
    dbus->mainloop = fcitx_instance_get_mainloop(instance);

    int retry = 0;

    // first init dbus
    if (!getenv("DISPLAY") && !getenv("DBUS_SESSION_BUS_ADDRESS")) {
        goto dbus_failed_no_conn;
    }

    /* try to get session dbus */
    while (1) {
        dbus->conn = dbus_bus_get_private(DBUS_BUS_SESSION, NULL);

        if (NULL == dbus->conn && retry < MAX_RETRY_TIMES) {
            retry ++;
            sleep(RETRY_INTERVAL * retry);
        } else {
            break;
        }
    }

    if (NULL == dbus->conn) {
        goto dbus_failed_no_conn;
    }

    if (!fcitx_dbus_setup_connection(dbus, dbus->conn)) {
        goto dbus_failed;
    }

    /* from here we know dbus connection is successful, now we need to register the service */
    dbus_connection_set_exit_on_disconnect(dbus->conn, FALSE);

    bool request_retry;
    bool doReplace = fcitx_instance_get_try_replace(instance);
    bool replaceCountdown = doReplace ? 3 : 0;

    do {
        request_retry = false;

        int replaceBit = doReplace ? DBUS_NAME_FLAG_REPLACE_EXISTING : 0;
        // request a name on the bus
        DBusError error;
        dbus_error_init(&error);
        int ret = dbus_bus_request_name(dbus->conn, "org.fcitx.Fcitx",
                                        DBUS_NAME_FLAG_ALLOW_REPLACEMENT | DBUS_NAME_FLAG_DO_NOT_QUEUE | replaceBit,
                                        &error);
        if (dbus_error_is_set(&error)) {
            fprintf(stderr, "%s %s", error.name, error.message);
        }
        dbus_error_free(&error);
        if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
            if (replaceCountdown > 0) {
                replaceCountdown --;
                // fcitx_utils_launch_tool("fcitx-remote", "-e");

                /* sleep for a while and retry */
                sleep(1);

                request_retry = true;
                continue;
            }

            goto dbus_failed_shutdown;
        }
    } while (request_retry);

    DBusObjectPathVTable controller_vtable = {NULL, &fcitx_dbus_controller_handler, NULL, NULL, NULL, NULL };
    dbus->objectRegistered = dbus_connection_register_object_path(dbus->conn, "/controller", &controller_vtable, dbus);
    dbus_connection_flush(dbus->conn);

    return dbus;
    // failed case need shutdown
dbus_failed_shutdown:
    fcitx_instance_shutdown(dbus->instance);
dbus_failed:
    // failed case doesn't need shutdown
    fcitx_dbus_destroy(dbus);
    return NULL;
    // failed case without connection
dbus_failed_no_conn:
    free(dbus);
    return NULL;
}

DBusHandlerResult
fcitx_dbus_filter(DBusConnection* connection, DBusMessage* msg, void* user_data)
{
    FCITX_UNUSED(connection);
    FcitxDBus* dbus = (FcitxDBus*) user_data;
    if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        fcitx_instance_shutdown(dbus->instance);
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal(msg, DBUS_INTERFACE_DBUS, "NameLost")) {
        const char* name;
        do {
            if (!dbus_message_get_args(msg, NULL,
                                       DBUS_TYPE_STRING, &name,
                                       DBUS_TYPE_INVALID)) {
                break;
            }

            if (strcmp(name, "org.fcitx.Fcitx") == 0) {
                fcitx_instance_shutdown(dbus->instance);
                return DBUS_HANDLER_RESULT_HANDLED;
            }
        } while(0);
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


void fcitx_dbus_destroy(void* data)
{
    FcitxDBus* dbus = data;
    dbus->shutdown = true;
    if (dbus->objectRegistered) {
        dbus_connection_unregister_object_path(dbus->conn, "/controller");
    }
    if (dbus->wakeup_main) {
        fcitx_mainloop_remove_timeout_event(dbus->mainloop, dbus->wakeup_main);
    }

    dbus_connection_close(dbus->conn);
    dbus_connection_unref(dbus->conn);
    dbus_shutdown();
    free(dbus);
}

DBusConnection* fcitx_dbus_get_connection(FcitxDBus* self)
{
    return self->conn;
}
