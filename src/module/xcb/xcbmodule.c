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

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "fcitx-xcb-internal.h"

typedef union {
    /* All XKB events share these fields. */
    struct {
        uint8_t response_type;
        uint8_t xkbType;
        uint16_t sequence;
        xcb_timestamp_t time;
        uint8_t deviceID;
    } any;
    xcb_xkb_new_keyboard_notify_event_t new_keyboard_notify;
    xcb_xkb_map_notify_event_t map_notify;
    xcb_xkb_state_notify_event_t state_notify;
} _xkb_event;

typedef struct _FcitxXCB
{
    FcitxInstance* instance;
    FcitxDict* conns;
    FcitxHandlerTable* table;
    FcitxInputContextManager* icManager;
} FcitxXCB;

typedef struct _FcitxXCBConnection
{
    xcb_connection_t* conn;
    int screen;
    FcitxIOEvent* event;
    FcitxXCB* xcb;
    char* name;
    xcb_atom_t atom;
    xcb_window_t server_window;
    FcitxInputContextFocusGroup* group;
    xcb_atom_t xkbRulesNamesAtom;
    xcb_window_t root;
    uint8_t xkbFirstEvent;
    bool hasXKB;
    int32_t coreDeviceId;
    struct xkb_context* context;
    struct xkb_keymap* keymap;
    struct xkb_state* state;
} FcitxXCBConnection;

typedef struct _FcitxXCBCallbackClosure
{
    FcitxCallback callback;
    void* userData;
} FcitxXCBCallbackClosure;

typedef void (*FcitxXCBConnectionCreatedCallback)(const char* name,
                                                  xcb_connection_t* conn,
                                                  int screen,
                                                  FcitxInputContextFocusGroup* group,
                                                  void* userData);
typedef void (*FcitxXCBConnectionClosedCallback)(const char* name, xcb_connection_t* conn, void* userData);
typedef bool (*FcitxXCBEventFilterCallback)(xcb_connection_t* conn, xcb_generic_event_t*, void* userData);

static void* fcitx_xcb_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_xcb_destroy(void* data);
static void fcitx_xcb_open_connection(FcitxXCB* xcb, const char* name);
static bool fcitx_xcb_fitler_event(xcb_connection_t* conn, xcb_generic_event_t* event, void* data);
static void fcitx_xcb_update_keymap(FcitxXCBConnection* fconn);

static void* const connection_created = (void*) fcitx_xcb_on_connection_created;
static void* const connection_closed = (void*) fcitx_xcb_on_connection_closed;

FCITX_DEFINE_ADDON(fcitx_xcb, module, FcitxAddonAPICommon) = {
    .init = fcitx_xcb_init,
    .destroy = fcitx_xcb_destroy,
    .registerCallback = fcitx_xcb_register_functions
};

void fcitx_xcb_io_callback(FcitxIOEvent* _event, int fd, unsigned int flag, void* data)
{
    FCITX_UNUSED(_event);
    FCITX_UNUSED(fd);
    FCITX_UNUSED(flag);

    FcitxXCBConnection* fconn = data;
    xcb_generic_event_t* event;

    if (xcb_connection_has_error(fconn->conn)) {
        fcitx_dict_remove_by_str(fconn->xcb->conns, fconn->name, NULL);
        return;
    }

    while ((event = xcb_poll_for_event(fconn->conn))) {
        FcitxXCBCallbackClosure* p;
        for (int id = fcitx_handler_table_first_id(fconn->xcb->table, sizeof(void*), &fconn), nextid;
             (p = fcitx_handler_table_get_by_id(fconn->xcb->table, id));
             id = nextid) {
            nextid = fcitx_handler_table_next_id(fconn->xcb->table, p);
            if (((FcitxXCBEventFilterCallback) p->callback)(fconn->conn, event, p->userData)) {
                break;
            }
        }

        free(event);
    }

}

void fcitx_xcb_connection_close(void* data)
{
    FcitxXCBConnection* fconn = data;

    FcitxXCBCallbackClosure* p;
    for (int id = fcitx_handler_table_first_id(fconn->xcb->table, sizeof(void*), &connection_closed), nextid;
            (p = fcitx_handler_table_get_by_id(fconn->xcb->table, id));
            id = nextid) {
        nextid = fcitx_handler_table_next_id(fconn->xcb->table, p);
        ((FcitxXCBConnectionClosedCallback) p->callback)(fconn->name, fconn->conn, p->userData);
    }

    FcitxInputContextFocusGroup* group = fconn->group;
    fconn->group = NULL;
    fcitx_input_context_focus_group_free(group);

    FcitxXCB* xcb = fconn->xcb;
    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(xcb->instance);
    fcitx_mainloop_remove_io_event(mainloop, fconn->event);
    xcb_disconnect(fconn->conn);

    free(fconn->name);
    free(fconn);
}

void* fcitx_xcb_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxXCB* xcb = fcitx_utils_new(FcitxXCB);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    FcitxInputContextManager* icManager = fcitx_addon_manager_get_property(manager, "icmanager");
    xcb->instance = instance;
    xcb->icManager = icManager;
    xcb->conns = fcitx_dict_new(fcitx_xcb_connection_close);
    xcb->table = fcitx_handler_table_new(sizeof(FcitxXCBCallbackClosure), NULL);

    fcitx_xcb_open_connection(xcb, NULL);

    return xcb;
}

void fcitx_xcb_open_connection(FcitxXCB* xcb, const char* name)
{
    if (!name) {
        name = getenv("DISPLAY");
    }

    if (!name) {
        return;
    }

    if (fcitx_dict_lookup_by_str(xcb->conns, name, NULL)) {
        return;
    }

    int screenp;
    xcb_connection_t* conn = xcb_connect(name, &screenp);
    // memory error
    if (!conn) {
        return;
    }
    if (xcb_connection_has_error(conn)) {
        goto _xcb_init_error;
    }

    xcb_atom_t atom;
    xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(conn, false, strlen("_FCITX_SERVER"), "_FCITX_SERVER");
    xcb_intern_atom_reply_t* atom_reply = xcb_intern_atom_reply(conn, atom_cookie, NULL);
    if (atom_reply) {
        atom = atom_reply->atom;
        free(atom_reply);
    } else {
        goto _xcb_init_error;
    }
    xcb_window_t w = xcb_generate_id (conn);
    xcb_screen_t* screen = xcb_aux_get_screen(conn, screenp);
    xcb_create_window (conn, XCB_COPY_FROM_PARENT, w, screen->root,
                       0, 0, 1, 1, 1,
                       XCB_WINDOW_CLASS_INPUT_OUTPUT,
                       screen->root_visual,
                       0, NULL);

    xcb_set_selection_owner(conn, w, atom, XCB_CURRENT_TIME);

    FcitxXCBConnection* fconn = fcitx_utils_new(FcitxXCBConnection);
    fconn->xcb = xcb;
    fconn->conn = conn;
    fconn->screen = screenp;
    fconn->name = fcitx_utils_strdup(name);
    fconn->atom = atom;
    fconn->root = screen->root;
    fconn->server_window = w;
    int fd = xcb_get_file_descriptor(conn);
    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(xcb->instance);
    fconn->event = fcitx_mainloop_register_io_event(mainloop, fd, FIOEF_IN, fcitx_xcb_io_callback, NULL, fconn);

    const xcb_query_extension_reply_t *reply = xcb_get_extension_data(conn, &xcb_xkb_id);
    if (reply && reply->present) {
        fconn->xkbFirstEvent = reply->first_event;
        xcb_xkb_use_extension_cookie_t xkb_query_cookie;
        xcb_xkb_use_extension_reply_t *xkb_query;

        xkb_query_cookie = xcb_xkb_use_extension(conn, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION);
        xkb_query = xcb_xkb_use_extension_reply(conn, xkb_query_cookie, NULL);

        if (xkb_query && xkb_query->supported) {
            fconn->coreDeviceId = xkb_x11_get_core_keyboard_device_id(conn);

            const uint16_t required_map_parts = (XCB_XKB_MAP_PART_KEY_TYPES |
                                                 XCB_XKB_MAP_PART_KEY_SYMS |
                                                 XCB_XKB_MAP_PART_MODIFIER_MAP |
                                                 XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS |
                                                 XCB_XKB_MAP_PART_KEY_ACTIONS |
                                                 XCB_XKB_MAP_PART_KEY_BEHAVIORS |
                                                 XCB_XKB_MAP_PART_VIRTUAL_MODS |
                                                 XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP);

            const uint16_t required_events = (XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
                                              XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                                              XCB_XKB_EVENT_TYPE_STATE_NOTIFY);

            // XKB events are reported to all interested clients without regard
            // to the current keyboard input focus or grab state
            xcb_void_cookie_t select = xcb_xkb_select_events_checked(conn,
                                                                     XCB_XKB_ID_USE_CORE_KBD,
                                                                     required_events,
                                                                     0,
                                                                     required_events,
                                                                     required_map_parts,
                                                                     required_map_parts,
                                                                     0);
            xcb_generic_error_t *error = xcb_request_check(conn, select);
            if (error) {
                free(error);
            } else {
                fconn->hasXKB = true;
                fcitx_xcb_update_keymap(fconn);
            }
        }

        free(xkb_query);
    }

    fcitx_dict_insert_by_str(xcb->conns, name, fconn, false);

    // create a focus group for display server
    FcitxInputContextFocusGroup* group = fcitx_input_context_manager_create_focus_group(xcb->icManager);
    fconn->group = group;

    fcitx_xcb_add_event_filter(xcb, name, fcitx_xcb_fitler_event, fconn);

    // call on create callback
    FcitxXCBCallbackClosure* p;
    for (int id = fcitx_handler_table_first_id(fconn->xcb->table, sizeof(void*), &connection_created), nextid;
            (p = fcitx_handler_table_get_by_id(fconn->xcb->table, id));
            id = nextid) {
        nextid = fcitx_handler_table_next_id(fconn->xcb->table, p);
        ((FcitxXCBConnectionCreatedCallback) p->callback)(fconn->name, fconn->conn, fconn->screen, fconn->group, p->userData);
    }

    return;

_xcb_init_error:
    xcb_disconnect(conn);

// _xcb_open_connection_error:
    return;
}

void fcitx_xcb_destroy(void* data)
{
    FcitxXCB* xcb = data;
    fcitx_dict_free(xcb->conns);
    fcitx_handler_table_free(xcb->table);
    free(xcb);
}

xcb_connection_t* fcitx_xcb_get_connection(FcitxXCB* self, const char* name)
{
    if (name == NULL) {
        FcitxDictData* data = fcitx_dict_first(self->conns);
        if (data) {
            FcitxXCBConnection* fconn = data->data;
            return fconn->conn;
        }
        return NULL;
    }

    FcitxXCBConnection* fconn = NULL;
    if (fcitx_dict_lookup_by_str(self->conns, name, &fconn)) {
        return fconn->conn;
    }
    return NULL;
}

int fcitx_xcb_on_connection_created(FcitxXCB* self, void* _callback, void* userdata)
{
    FcitxXCBCallbackClosure closure;
    closure.callback = _callback;
    closure.userData = userdata;
    int result = fcitx_handler_table_append(self->table, sizeof(void*), &connection_created, &closure);

    // call it on all existing connection
    for(FcitxDictData* data = fcitx_dict_first(self->conns);
        data;
        data = fcitx_dict_data_next(data)) {
        FcitxXCBConnection* fconn = data->data;
        FcitxXCBConnectionCreatedCallback callback = _callback;
        callback(fconn->name, fconn->conn, fconn->screen, fconn->group, userdata);
    }

    return result;
}

int fcitx_xcb_on_connection_closed(FcitxXCB* self, void* callback, void* userdata)
{
    FcitxXCBCallbackClosure closure;
    closure.callback = callback;
    closure.userData = userdata;
    return fcitx_handler_table_append(self->table, sizeof(void*), &connection_closed, &closure);
}

int fcitx_xcb_add_event_filter(FcitxXCB* self, const char* name, void* callback, void* userdata)
{
    FcitxXCBConnection* fconn = NULL;
    if (!fcitx_dict_lookup_by_str(self->conns, name, &fconn)) {
        return FCITX_OBJECT_POOL_INVALID_ID;
    }

    FcitxXCBCallbackClosure closure;
    closure.callback = callback;
    closure.userData = userdata;

    return fcitx_handler_table_append(self->table, sizeof(void*), &fconn, &closure);
}

void fcitx_xcb_remove_watcher(FcitxXCB* self, int id)
{
    fcitx_handler_table_remove_by_id_full(self->table, id);
}

char* fcitx_xcb_get_xkb_rules_names(FcitxXCB* self, const char* display, int* pLength)
{
    do {
        FcitxXCBConnection* fconn = NULL;
        if (!fcitx_dict_lookup_by_str(self->conns, display, &fconn)) {
            break;
        }

        if (!fconn->xkbRulesNamesAtom) {
            xcb_intern_atom_cookie_t cookie = xcb_intern_atom(fconn->conn, true, strlen("_XKB_RULES_NAMES"), "_XKB_RULES_NAMES");
            xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(fconn->conn, cookie, NULL);
            if (reply) {
                fconn->xkbRulesNamesAtom = reply->atom;
            }
            free(reply);
        }

        if (!fconn->xkbRulesNamesAtom) {
            break;
        }

        xcb_get_property_cookie_t get_prop_cookie = xcb_get_property(fconn->conn, false, fconn->root, fconn->xkbRulesNamesAtom, XCB_ATOM_STRING, 0, 1024);
        xcb_get_property_reply_t* reply = xcb_get_property_reply(fconn->conn, get_prop_cookie, NULL);

        if (!reply) {
            break;
        }

        if (reply->type != XCB_ATOM_STRING || reply->bytes_after > 0 || reply->format != 8) {
            break;
        }

        void* data = xcb_get_property_value(reply);
        int length = xcb_get_property_value_length(reply);
        char* result = fcitx_utils_malloc(length);
        if (result) {
            memcpy(result, data, length);
            if (pLength) {
                *pLength = length;
            }
        }

        free(reply);

        return result;
    } while(0);

    return NULL;
}


bool fcitx_xcb_fitler_event(xcb_connection_t* conn, xcb_generic_event_t* event, void* data)
{
    FCITX_UNUSED(conn);
    FcitxXCBConnection* fconn = data;

    uint8_t response_type = event->response_type & ~0x80;
    if (response_type == fconn->xkbFirstEvent) {
        _xkb_event* xkbEvent = (_xkb_event*) event;
        if (xkbEvent->any.deviceID == fconn->coreDeviceId) {
            switch(xkbEvent->any.xkbType) {
                case XCB_XKB_STATE_NOTIFY: {
                        xcb_xkb_state_notify_event_t* state = &xkbEvent->state_notify;
                        xkb_state_update_mask(fconn->state,
                                              state->baseMods,
                                              state->latchedMods,
                                              state->lockedMods,
                                              state->baseGroup,
                                              state->latchedGroup,
                                              state->lockedGroup);
                    }
                    return true;
                    break;
                case XCB_XKB_MAP_NOTIFY: {
                        fcitx_xcb_update_keymap(fconn);
                    }
                    return true;
                    break;
                case XCB_XKB_NEW_KEYBOARD_NOTIFY: {
                        xcb_xkb_new_keyboard_notify_event_t *ev = &xkbEvent->new_keyboard_notify;
                        if (ev->changed & XCB_XKB_NKN_DETAIL_KEYCODES) {
                            fcitx_xcb_update_keymap(fconn);
                        }
                    }
                    break;
            }
        }
    }
    return false;
}

void fcitx_xcb_update_keymap(FcitxXCBConnection* fconn)
{
    if (!fconn->context) {
        fconn->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        xkb_context_set_log_level(fconn->context, XKB_LOG_LEVEL_CRITICAL);
    }

    if (!fconn->context) {
        return;
    }

    xkb_keymap_unref(fconn->keymap);
    fconn->keymap = NULL;

    struct xkb_state* new_state = NULL;
    if (fconn->hasXKB) {
        fconn->keymap = xkb_x11_keymap_new_from_device(fconn->context, fconn->conn, fconn->coreDeviceId, 0);
        if (fconn->keymap) {
            new_state = xkb_x11_state_new_from_device(fconn->keymap, fconn->conn, fconn->coreDeviceId);
        }
    }

    if (!fconn->keymap) {
        struct xkb_rule_names xkbNames;

        int length;
        char* nameString = fcitx_xcb_get_xkb_rules_names(fconn->xcb, fconn->name, &length);
        if (nameString) {
            char *names[5] = { 0, 0, 0, 0, 0 };
            char *p = nameString, *end = p + length;
            int i = 0;
            // The result from xcb_get_property_value() is not necessarily \0-terminated,
            // we need to make sure that too many or missing '\0' symbols are handled safely.
            do {
                uint len = strnlen(p, length);
                names[i++] = p;
                p += len + 1;
                length -= len + 1;
            } while (p < end || i < 5);

            xkbNames.rules = names[0];
            xkbNames.model = names[1];
            xkbNames.layout = names[2];
            xkbNames.variant = names[3];
            xkbNames.options = names[4];

            fconn->keymap = xkb_keymap_new_from_names(fconn->context, &xkbNames, XKB_KEYMAP_COMPILE_NO_FLAGS);
        }

        if (!fconn->keymap) {
            memset(&xkbNames, 0, sizeof(xkbNames));
            fconn->keymap = xkb_keymap_new_from_names(fconn->context, &xkbNames, XKB_KEYMAP_COMPILE_NO_FLAGS);
        }

    }
    if (fconn->keymap) {
        new_state = xkb_state_new(fconn->keymap);
    }

    xkb_state_unref(fconn->state);
    fconn->state = new_state;
}

struct xkb_state* fcitx_xcb_get_xkb_state(FcitxXCB* self, const char* display)
{
    FcitxXCBConnection* fconn = NULL;
    if (!fcitx_dict_lookup_by_str(self->conns, display, &fconn)) {
        return NULL;
    }

    return fconn->state;
}
