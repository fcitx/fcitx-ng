/*
 * Copyright (C) 2012~2015 by CSSlayer
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
#include "module/dbus/fcitx-dbus.h"
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"

#define FCITX_INPUTMETHOD_DBUS_INTERFACE "org.fcitx.Fcitx.InputMethod2"
#define FCITX_INPUTCONTEXT_DBUS_INTERFACE "org.fcitx.Fcitx.InputContext2"

const char * inputmethod_introspection_xml =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
    "<node>"
    "<interface name=\"" DBUS_INTERFACE_INTROSPECTABLE "\">"
    "<method name=\"Introspect\">"
    "<arg name=\"data\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "<interface name=\"" FCITX_INPUTMETHOD_DBUS_INTERFACE "\">"
    "<method name=\"CreateIC\">"
    "<arg name=\"parameter\" direction=\"in\" type=\"a{sv}\"/>"
    "<arg name=\"icid\" direction=\"out\" type=\"i\"/>"
    "<arg name=\"uuid\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "</node>";

const char * inputcontext_introspection_xml =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
    "<node>"
    "<interface name=\"" DBUS_INTERFACE_INTROSPECTABLE "\">"
    "<method name=\"Introspect\">"
    "<arg name=\"data\" direction=\"out\" type=\"s\"/>"
    "</method>"
    "</interface>"
    "<interface name=\"" FCITX_INPUTCONTEXT_DBUS_INTERFACE "\">"
    "<method name=\"FocusIn\">"
    "</method>"
    "<method name=\"FocusOut\">"
    "</method>"
    "<method name=\"Reset\">"
    "</method>"
    "<method name=\"CommitPreedit\">"
    "</method>"
    "<method name=\"SetCursorRect\">"
    "<arg name=\"x\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"y\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"w\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"h\" direction=\"in\" type=\"i\"/>"
    "</method>"
    "<method name=\"SetCapacity\">"
    "<arg name=\"caps\" direction=\"in\" type=\"u\"/>"
    "</method>"
    "<method name=\"SetSurroundingText\">"
    "<arg name=\"text\" direction=\"in\" type=\"s\"/>"
    "<arg name=\"cursor\" direction=\"in\" type=\"u\"/>"
    "<arg name=\"anchor\" direction=\"in\" type=\"u\"/>"
    "</method>"
    "<method name=\"SetSurroundingTextPosition\">"
    "<arg name=\"cursor\" direction=\"in\" type=\"u\"/>"
    "<arg name=\"anchor\" direction=\"in\" type=\"u\"/>"
    "</method>"
    "<method name=\"DestroyIC\">"
    "</method>"
    "<method name=\"ProcessKeyEvent\">"
    "<arg name=\"keyval\" direction=\"in\" type=\"u\"/>"
    "<arg name=\"keycode\" direction=\"in\" type=\"u\"/>"
    "<arg name=\"state\" direction=\"in\" type=\"u\"/>"
    "<arg name=\"type\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"time\" direction=\"in\" type=\"u\"/>"
    "<arg name=\"ret\" direction=\"out\" type=\"i\"/>"
    "</method>"
    "<signal name=\"CommitString\">"
    "<arg name=\"str\" type=\"s\"/>"
    "</signal>"
    "<signal name=\"UpdatePreedit\">"
    "<arg name=\"str\" type=\"s\"/>"
    "<arg name=\"cursorpos\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"DeleteSurroundingText\">"
    "<arg name=\"offset\" type=\"i\"/>"
    "<arg name=\"nchar\" type=\"u\"/>"
    "</signal>"
    "<signal name=\"UpdateFormattedPreedit\">"
    "<arg name=\"str\" type=\"a(si)\"/>"
    "<arg name=\"cursorpos\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"UpdateClientSideUI\">"
    "<arg name=\"auxup\" type=\"s\"/>"
    "<arg name=\"auxdown\" type=\"s\"/>"
    "<arg name=\"preedit\" type=\"s\"/>"
    "<arg name=\"candidatewords\" type=\"ass\"/>"
    "<arg name=\"imname\" type=\"s\"/>"
    "<arg name=\"cursorpos\" type=\"i\"/>"
    "</signal>"
    "<signal name=\"ForwardKey\">"
    "<arg name=\"keyval\" type=\"u\"/>"
    "<arg name=\"state\" type=\"u\"/>"
    "<arg name=\"keycode\" type=\"u\"/>"
    "<arg name=\"type\" type=\"i\"/>"
    "</signal>"
    "</interface>"
    "</node>";

typedef struct _FcitxIPC
{
    FcitxInstance* instance;
    DBusConnection* conn;
    dbus_bool_t objectRegistered;
} FcitxIPC;

static void* fcitx_ipc_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_ipc_destroy(void* data);

FCITX_DEFINE_ADDON(fcitx_ipc, frontend, FcitxAddonAPICommon) = {
    .init = fcitx_ipc_init,
    .destroy = fcitx_ipc_destroy
};

DBusHandlerResult fcitx_ipc_inputmethod_handler(DBusConnection *connection, DBusMessage *msg, void *user_data)
{
    FCITX_UNUSED(user_data);    
    // FcitxIPC* ipc= user_data;
    DBusMessage* reply = NULL;
    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        reply = dbus_message_new_method_return(msg);
        dbus_message_append_args(reply, DBUS_TYPE_STRING, &inputmethod_introspection_xml, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTMETHOD_DBUS_INTERFACE, "CreateIC")) {
        reply = dbus_message_new_method_return(msg);
        dbus_connection_send(connection, reply, NULL);
    }

    if (reply) {
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        dbus_connection_flush(connection);
        result = DBUS_HANDLER_RESULT_HANDLED;
    }

    return result;
}

void* fcitx_ipc_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxIPC* ipc = fcitx_utils_new(FcitxIPC);
    ipc->conn = fcitx_dbus_invoke_get_connection(manager);

    DBusObjectPathVTable controller_vtable = {NULL, &fcitx_ipc_inputmethod_handler, NULL, NULL, NULL, NULL };
    if (!dbus_connection_register_object_path(ipc->conn, "/inputmethod", &controller_vtable, ipc)) {
        goto register_object_failed;
    }

    return ipc;
register_object_failed:
    free(ipc);
    return NULL;
}

void fcitx_ipc_destroy(void* data)
{
    FcitxIPC* ipc = data;
    dbus_connection_unregister_object_path(ipc->conn, "/inputmethod");
    free(ipc);
}
