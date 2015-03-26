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
#include <xcb-imdkit/imdkit.h>
#include <xkbcommon/xkbcommon.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "module/xcb/fcitx-xcb.h"
#include "xim-config.h"

typedef struct _FcitxXIM
{
    FcitxAddonManager* manager;
    FcitxDict* connFilterId;
    int connCreatedId;
    int connClosedId;
    FcitxInputContextManager* icManager;
} FcitxXIM;

typedef struct _FcitxXIMServer
{
    xcb_window_t window;
    FcitxXIM* xim;
    xcb_im_t* im;
    int id;
    xcb_connection_t* conn;
    FcitxInputContextFocusGroup* group;
    char* name;
} FcitxXIMServer;


static uint32_t style_array[] = {
    XCB_IM_PreeditPosition | XCB_IM_StatusArea, //OverTheSpot
    XCB_IM_PreeditPosition | XCB_IM_StatusNothing,      //OverTheSpot
    XCB_IM_PreeditPosition | XCB_IM_StatusNone, //OverTheSpot
    XCB_IM_PreeditNothing | XCB_IM_StatusNothing,       //Root
    XCB_IM_PreeditNothing | XCB_IM_StatusNone,  //Root
};
static char* encoding_array[] = {
    "COMPOUND_TEXT",
};

static xcb_im_encodings_t encodings = {
    1, encoding_array
};

static xcb_im_styles_t styles = {
    5, style_array
};

const char* guess_server_name()
{
    char* env = getenv("XMODIFIERS");
    if (env && fcitx_utils_string_starts_with(env, "@im=")) {
        return env + 4; // length of "@im="
    }

    return "fcitx";
}

static void* fcitx_xim_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_xim_destroy(void* data);

FCITX_DEFINE_ADDON(fcitx_xim, frontend, FcitxAddonAPIFrontend) = {
    .common = {
        .init = fcitx_xim_init,
        .destroy = fcitx_xim_destroy
    }
};


void callback(xcb_im_t* im, xcb_im_client_t* client, xcb_im_input_context_t* xic,
              const xcb_im_packet_header_fr_t* hdr,
              void* frame, void* arg, void* user_data)
{
    FCITX_UNUSED(im);
    FCITX_UNUSED(client);
    FCITX_UNUSED(hdr);
    FCITX_UNUSED(frame);
    FCITX_UNUSED(arg);

    FcitxXIMServer* server = user_data;
    FcitxInputContext* ic = NULL;
    if (xic && hdr->major_opcode != XIM_CREATE_IC) {
        ic = xcb_im_input_context_get_data(xic);
    }

    switch (hdr->major_opcode) {
        case XIM_CREATE_IC:
            ic = fcitx_input_context_new(server->xim->icManager, fcitx_xim_frontend.frontendId);
            xcb_im_input_context_set_data(xic, ic, NULL);
            fcitx_input_context_set_focus_group(ic, FICFG_Local, server->group);
            break;
        case XIM_DESTROY_IC:
            fcitx_input_context_destroy(ic);
            break;
        case XIM_SET_IC_VALUES:
            // kinds of like notification for position moving
            break;
        case XIM_FORWARD_EVENT:
        {
            struct xkb_state* xkbState = fcitx_xcb_invoke_get_xkb_state(server->xim->manager, server->name);
            if (!xkbState) {
                break;
            }
            xcb_key_press_event_t* xevent = arg;
            FcitxKeyEvent event;
            event.isRelease = (xevent->response_type & ~0x80) != XCB_KEY_RELEASE;
            event.keyCode = xevent->detail;
            event.rawKey.sym = xkb_state_key_get_one_sym(xkbState, xevent->detail);
            event.rawKey.state = xevent->state;
            event.key = event.rawKey;
            if (!fcitx_input_context_process_key_event(ic, &event)) {
                xcb_im_forward_event(im, xic, xevent);
            }
            break;
        }
        case XIM_RESET_IC:
            fcitx_input_context_reset(ic);
            break;
        case XIM_SET_IC_FOCUS:
            fcitx_input_context_focus_in(ic);
            break;
        case XIM_UNSET_IC_FOCUS:
            fcitx_input_context_focus_out(ic);
            break;
    }
}

bool fcitx_xim_xcb_event_fitler(xcb_connection_t* conn, xcb_generic_event_t* event, void* data)
{
    FCITX_UNUSED(conn);
    FcitxXIMServer* server = data;
    return xcb_im_filter_event(server->im, event);
}

void fcitx_xim_on_xcb_connection_created(const char* name,
                                         xcb_connection_t* conn,
                                         int defaultScreen,
                                         FcitxInputContextFocusGroup* group,
                                         void* data)
{
    FcitxXIM* self = data;
    FcitxXIMServer* server = fcitx_utils_new(FcitxXIMServer);

    xcb_screen_t* screen = xcb_aux_get_screen(conn, defaultScreen);
    xcb_window_t w = xcb_generate_id (conn);
    xcb_create_window (conn, XCB_COPY_FROM_PARENT, w, screen->root,
                       0, 0, 1, 1, 1,
                       XCB_WINDOW_CLASS_INPUT_OUTPUT,
                       screen->root_visual,
                       0, NULL);
    server->window = w;
    server->xim = self;
    server->group = group;
    server->name = strdup(name);
    server->im = xcb_im_create(conn,
                               defaultScreen,
                               w,
                               guess_server_name(),
                               XCB_IM_ALL_LOCALES,
                               &styles,
                               NULL,
                               NULL,
                               &encodings,
                               XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE,
                               callback,
                               server);
    server->conn = conn;


    if (xcb_im_open_im(server->im)) {
        server->id = fcitx_xcb_invoke_add_event_filter(self->manager, (void*) name, fcitx_xim_xcb_event_fitler, server);
        fcitx_dict_insert_by_str(self->connFilterId, name, server, false);
    } else {
        xcb_im_destroy(server->im);
        xcb_destroy_window(conn, server->window);
        free(server->name);
        free(server);
    }
}

void fcitx_xim_on_xcb_connection_closed(const char* name, xcb_connection_t* conn, void* data)
{
    FCITX_UNUSED(conn);
    FcitxXIM* self = data;
    FcitxXIMServer* server = NULL;
    if (fcitx_dict_lookup_by_str(self->connFilterId, name, &server)) {
        return;
    }

    server->conn = NULL;
    fcitx_dict_remove_by_str(self->connFilterId, name, NULL);
}

void fcitx_xim_server_destroy(void* data)
{
    FcitxXIMServer* server = data;
    FcitxXIM* self = server->xim;
    fcitx_xcb_invoke_remove_watcher(self->manager, server->id);
    xcb_im_close_im(server->im);
    xcb_im_destroy(server->im);
    // connection is going to close, what's the point of destroy window?
    if (server->conn) {
        xcb_destroy_window(server->conn, server->window);
    }
    free(server);
}

void* fcitx_xim_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxXIM* xim = fcitx_utils_new(FcitxXIM);
    FcitxInputContextManager* icmanager = fcitx_addon_manager_get_property(manager, "icmanager");

    xim->icManager = icmanager;
    xim->manager = manager;
    xim->connFilterId = fcitx_dict_new(fcitx_xim_server_destroy);

    xim->connCreatedId = fcitx_xcb_invoke_on_connection_created(manager, fcitx_xim_on_xcb_connection_created, xim);
    xim->connClosedId =fcitx_xcb_invoke_on_connection_closed(manager, fcitx_xim_on_xcb_connection_closed, xim);

    return xim;
}

void fcitx_xim_destroy(void* data)
{
    FcitxXIM* xim = data;
    fcitx_xcb_invoke_remove_watcher(xim->manager, xim->connCreatedId);
    fcitx_xcb_invoke_remove_watcher(xim->manager, xim->connClosedId);
    fcitx_dict_free(xim->connFilterId);
    free(xim);
}
