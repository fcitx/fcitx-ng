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
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include "fcitx-dbus.h"
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
    "<arg name=\"uuid\" direction=\"out\" type=\"ay\"/>"
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
    "<method name=\"SetCursorRect\">"
    "<arg name=\"x\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"y\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"w\" direction=\"in\" type=\"i\"/>"
    "<arg name=\"h\" direction=\"in\" type=\"i\"/>"
    "</method>"
    "<method name=\"SetCapability\">"
    "<arg name=\"cap\" direction=\"in\" type=\"u\"/>"
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
    "<arg name=\"time\" direction=\"in\" type=\"t\"/>"
    "<arg name=\"ret\" direction=\"out\" type=\"i\"/>"
    "</method>"
    "<signal name=\"CommitString\">"
    "<arg name=\"str\" type=\"s\"/>"
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
    FcitxInputContextManager* icManager;
} FcitxIPC;

static void* fcitx_ipc_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_ipc_destroy(void* data);
static bool fcitx_ipc_handle_event(void* self, FcitxEvent* event);

FCITX_DEFINE_ADDON(fcitx_ipc, frontend, FcitxAddonAPIFrontend) = {
    .common = {
        .init = fcitx_ipc_init,
        .destroy = fcitx_ipc_destroy
    },
    .handleEvent = fcitx_ipc_handle_event,
};

DBusHandlerResult fcitx_ipc_inputcontext_handler(DBusConnection *connection, DBusMessage *msg, void *user_data)
{
    FcitxIPC* ipc = user_data;
    DBusMessage* reply = NULL;
    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    const char* path = dbus_message_get_path(msg);
    uint32_t id = 0;
    FcitxInputContext* ic = NULL;
    if (!fcitx_utils_string_starts_with(path, "/inputcontext/")) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    } else {
        char* end;
        const char* idString = path + strlen("/inputcontext/");
        id = strtoul(idString, &end, 10);
        if (!(*idString != '\0' && end != idString && *end == '\0')) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        if (!(ic = fcitx_input_context_manager_get_input_context(ipc->icManager, id))) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }
    }

    DBusError error;
    dbus_error_init(&error);
    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        reply = dbus_message_new_method_return(msg);
        dbus_message_append_args(reply, DBUS_TYPE_STRING, &inputcontext_introspection_xml, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "FocusIn")) {
        fcitx_input_context_focus_in(ic);

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "FocusOut")) {
        fcitx_input_context_focus_out(ic);

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "Reset")) {
        fcitx_input_context_reset(ic);

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "SetCursorRect")) {
        int32_t x, y, w, h;
        if (dbus_message_get_args(msg, &error,
            DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y,
            DBUS_TYPE_INT32, &w, DBUS_TYPE_INT32, &h,
            DBUS_TYPE_INVALID)) {
            FcitxRect rect;
            rect.x1 = x;
            rect.x2 = x + w;
            rect.y1 = y;
            rect.y2 = y + h;
            fcitx_input_context_set_cursor_rect(ic, rect);
        }

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "SetCapability")) {
        uint32_t flag;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_UINT32, &flag, DBUS_TYPE_INVALID)) {
            fcitx_input_context_set_capability_flags(ic, flag);
        }

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "SetSurroundingText")) {
        char* text;
        uint32_t cursor, anchor;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_STRING, &text,  DBUS_TYPE_UINT32, &cursor, DBUS_TYPE_UINT32, &anchor, DBUS_TYPE_INVALID)) {
            fcitx_input_context_set_surrounding_text(ic, text, cursor, anchor);
        }

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "SetSurroundingTextPosition")) {
        uint32_t cursor, anchor;
        if (dbus_message_get_args(msg, &error,  DBUS_TYPE_UINT32, &cursor, DBUS_TYPE_UINT32, &anchor, DBUS_TYPE_INVALID)) {
            fcitx_input_context_set_surrounding_text(ic, NULL, cursor, anchor);
        }

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "DestroyIC")) {
        dbus_connection_unregister_object_path(connection, path);
        fcitx_input_context_destroy(ic);

        reply = dbus_message_new_method_return(msg);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTCONTEXT_DBUS_INTERFACE, "ProcessKeyEvent")) {
        uint32_t keyval, keycode, state, time;
        int ret, itype;
        if (dbus_message_get_args(msg, &error,
                                DBUS_TYPE_UINT32, &keyval,
                                DBUS_TYPE_UINT32, &keycode,
                                DBUS_TYPE_UINT32, &state,
                                DBUS_TYPE_INT32, &itype,
                                DBUS_TYPE_UINT64, &time,
                                DBUS_TYPE_INVALID)) {
            FcitxKeyEvent event;
            event.key.sym = keyval;
            event.key.state = state;
            event.keyCode = keycode;
            event.isRelease = itype != 0;
            event.rawKey.sym = keyval;
            event.rawKey.state = state;
            event.time = time;

            ret = fcitx_input_context_process_key_event(ic, &event);

            reply = dbus_message_new_method_return(msg);

            dbus_message_append_args(reply, DBUS_TYPE_INT32, &ret, DBUS_TYPE_INVALID);
        } else {
            reply = dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS, "");
        }
    }

    dbus_error_free(&error);
    if (reply) {
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        dbus_connection_flush(connection);
        result = DBUS_HANDLER_RESULT_HANDLED;
    }

    return result;
}

DBusHandlerResult fcitx_ipc_inputmethod_handler(DBusConnection *connection, DBusMessage *msg, void *user_data)
{
    FcitxIPC* ipc = user_data;
    DBusMessage* reply = NULL;
    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        reply = dbus_message_new_method_return(msg);
        dbus_message_append_args(reply, DBUS_TYPE_STRING, &inputmethod_introspection_xml, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, FCITX_INPUTMETHOD_DBUS_INTERFACE, "CreateIC")) {
        FcitxInputContext* ic = fcitx_input_context_new(ipc->icManager, fcitx_ipc_frontend.frontendId);
        uint32_t icid = fcitx_input_context_get_id(ic);

        DBusObjectPathVTable controller_vtable = {NULL, &fcitx_ipc_inputcontext_handler, NULL, NULL, NULL, NULL };

        char icPath[50];

        sprintf(icPath, "/inputcontext/%u", icid);

        if (dbus_connection_register_object_path(ipc->conn, icPath, &controller_vtable, ipc)) {
            uint8_t uuid[16];
            fcitx_input_context_get_uuid(ic, uuid);

            const uint8_t* vByte = uuid;

            reply = dbus_message_new_method_return(msg);
            dbus_message_append_args(reply,
                                    DBUS_TYPE_UINT32, &icid,
                                    DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &vByte, 16,
                                    DBUS_TYPE_INVALID);
        } else {
            fcitx_input_context_destroy(ic);
            reply = dbus_message_new_error(msg, DBUS_ERROR_NO_MEMORY, "register failed");
        }
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
    ipc->icManager = fcitx_addon_manager_get_property(manager, "icmanager");

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

DBusMessage* fcitx_ipc_new_signal(uint32_t id, const char* name)
{

    char icPath[50];

    sprintf(icPath, "/inputcontext/%u", id);
    DBusMessage* msg = dbus_message_new_signal(icPath, // object name of the signal
                                               FCITX_INPUTCONTEXT_DBUS_INTERFACE, // interface name of the signal
                                               name); // name of the signal

    return msg;
}

void fcitx_ipc_send_signal(FcitxIPC* self, uint32_t id, const char* name, int firstArgType, ...)
{

    char icPath[50];
    va_list va;
    va_start(va, firstArgType);

    sprintf(icPath, "/inputcontext/%u", id);
    DBusMessage* msg = fcitx_ipc_new_signal(id, name);
    if (msg) {
        dbus_message_append_args_valist(msg, firstArgType, va);

        dbus_connection_send(self->conn, msg, NULL);
        dbus_connection_flush(self->conn);
        dbus_message_unref(msg);
    }

    va_end(va);
}

void fcitx_ipc_commit_string(FcitxIPC* self, uint32_t id, const char* str)
{
    if (!fcitx_utf8_check_string(str))
        return;

    fcitx_ipc_send_signal(self, id, "CommitString", DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
}

void fcitx_ipc_delete_surrounding_text(FcitxIPC* self, uint32_t id, int offset, unsigned int length)
{
    fcitx_ipc_send_signal(self, id, "DeleteSurroundingText", DBUS_TYPE_INT32, &offset, DBUS_TYPE_UINT32, &length, DBUS_TYPE_INVALID);
}

void fcitx_ipc_forward_key(FcitxIPC* self, uint32_t id, FcitxKey key, int32_t type)
{
    uint32_t keyval = (uint32_t) key.sym;
    uint32_t keystate = (uint32_t) key.state;
    fcitx_ipc_send_signal(self, id, "ForwardKey", DBUS_TYPE_UINT32, &keyval, DBUS_TYPE_UINT32, &keystate, DBUS_TYPE_INT32, &type, DBUS_TYPE_INVALID);
}

bool fcitx_ipc_handle_event(void* _self, FcitxEvent* event)
{
    FcitxIPC* self = _self;
    switch (event->type) {
        case ET_InputContextCommitString:
            {
                FcitxInputContextCommitStringEvent* commitStringEvent = (FcitxInputContextCommitStringEvent*) event;
                fcitx_ipc_commit_string(self, commitStringEvent->id, commitStringEvent->commitString);
            }
            break;
        case ET_InputContextDeleteSurroundingText:
            {
                FcitxInputContextDeleteSurroundingEvent* deleteSurroundingTextEvent =
                (FcitxInputContextDeleteSurroundingEvent*) event;
                fcitx_ipc_delete_surrounding_text(self, deleteSurroundingTextEvent->id, deleteSurroundingTextEvent->offset, deleteSurroundingTextEvent->length);
            }
            break;
        case ET_InputContextForwardKey:
            {
                FcitxInputContextKeyEvent* forwardKeyEvent =
                (FcitxInputContextKeyEvent*) event;
                fcitx_ipc_forward_key(self, forwardKeyEvent->id, forwardKeyEvent->detail.key, !!forwardKeyEvent->detail.isRelease);
            }
            break;
        case ET_InputContextUpdatePreedit:
            {
                // TODO
            }
            break;
        default:
            return false;
    }
    return true;
}
