#include <dbus/dbus.h>
#include <unistd.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"

typedef struct _FcitxDBus
{
    FcitxInstance* instance;
    FcitxMainLoop* mainloop;
    DBusConnection* conn;
} FcitxDBus;

const int RETRY_INTERVAL = 1;
const int MAX_RETRY_TIMES = 5;

static void* fcitx_dbus_init(FcitxAddonManager* manager);
static void fcitx_dbus_destroy(void* data);
static DBusHandlerResult fcitx_dbus_filter(DBusConnection* connection, DBusMessage* msg, void* user_data);

FCITX_DEFINE_ADDON(dbus, module, FcitxAddonAPICommon) = {
    .init = fcitx_dbus_init,
    .destroy = fcitx_dbus_destroy
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

bool fcitx_dbus_setup_connection(FcitxDBus* dbus, DBusConnection* conn)
{
    if (!dbus_connection_add_filter(conn, fcitx_dbus_filter, dbus, NULL))
        return false;

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

    return true;
}

void* fcitx_dbus_init(FcitxAddonManager* manager)
{
    FcitxDBus* dbus = fcitx_utils_new(FcitxDBus);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    dbus->instance = instance;
    dbus->mainloop = fcitx_instance_get_mainloop(instance);

    int retry = 0;

    DBusConnection* conn;
    // first init dbus
    do {
        if (!getenv("DISPLAY") && !getenv("DBUS_SESSION_BUS_ADDRESS")) {
            break;
        }

        /* try to get session dbus */
        while (1) {
            conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);

            if (NULL == conn && retry < MAX_RETRY_TIMES) {
                retry ++;
                sleep(RETRY_INTERVAL * retry);
            } else {
                break;
            }
        }

        if (NULL == conn) {
            break;
        }

        if (!fcitx_dbus_setup_connection(dbus, conn)) {
            dbus_connection_unref(conn);
            conn = NULL;
        }

        /* from here we know dbus connection is successful, now we need to register the service */
        dbus_connection_set_exit_on_disconnect(conn, FALSE);
        dbus->conn = conn;

        bool request_retry;
        bool doReplace = true;
        bool replaceCountdown = doReplace ? 3 : 0;

        do {
            request_retry = false;

            int replaceBit = doReplace ? DBUS_NAME_FLAG_REPLACE_EXISTING : 0;
            // request a name on the bus
            int ret = dbus_bus_request_name(conn, "org.fcitx.Fcitx",
                                            DBUS_NAME_FLAG_ALLOW_REPLACEMENT | DBUS_NAME_FLAG_DO_NOT_QUEUE | replaceBit,
                                            NULL);
            if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
                if (replaceCountdown > 0) {
                    replaceCountdown --;
                    // fcitx_utils_launch_tool("fcitx-remote", "-e");

                    /* sleep for a while and retry */
                    sleep(1);

                    request_retry = true;
                    continue;
                }

                fcitx_instance_shutdown(dbus->instance);
                dbus_connection_unref(conn);
                free(dbus);
                return NULL;
            } else {
                dbus_bus_request_name(conn, "org.fcitx.Fcitx", DBUS_NAME_FLAG_DO_NOT_QUEUE, NULL);
            }
        } while (request_retry);

        dbus_connection_flush(conn);
    } while(0);


    /* from here we know dbus connection is successful, now we need to register the service */
    dbus_connection_set_exit_on_disconnect(conn, FALSE);

    return dbus;
}

DBusHandlerResult
fcitx_dbus_filter(DBusConnection* connection, DBusMessage* msg, void* user_data)
{
    FCITX_UNUSED(connection);
    FcitxDBus* dbus = (FcitxDBus*) user_data;
    if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        fcitx_instance_shutdown(dbus->instance);
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "NameLost")) {
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
    free(dbus);
}
