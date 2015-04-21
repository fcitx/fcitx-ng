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
#include <xcb-imdkit/encoding.h>
#include <xkbcommon/xkbcommon.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "fcitx-xcb.h"
#include "xim-config.h"

typedef struct _FcitxXIM
{
    FcitxAddonManager* manager;
    FcitxDict* connFilterId;
    int connCreatedId;
    int connClosedId;
    UT_array feedbackBuffer;
    FcitxInputContextManager* icManager;
    int32_t frontendDataId;
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
    xcb_window_t root;
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
static bool fcitx_xim_handle_event(void* data, FcitxEvent* event);

FCITX_DEFINE_ADDON(fcitx_xim, frontend, FcitxAddonAPIFrontend) = {
    .common = {
        .init = fcitx_xim_init,
        .destroy = fcitx_xim_destroy,
    },
    .handleEvent = fcitx_xim_handle_event
};


typedef struct _FcitxXIMICData
{
    FcitxInputContext* ic;
    FcitxXIMServer* server;
    bool preeditStarted;
    uint32_t lastPreeditLength;
} FcitxXIMICData;

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
    if (!xic) {
        return;
    }
    if (hdr->major_opcode != XCB_XIM_CREATE_IC) {
        FcitxXIMICData* icData = xcb_im_input_context_get_data(xic);
        ic = icData->ic;
    }

    switch (hdr->major_opcode) {
        case XCB_XIM_CREATE_IC:
            ic = fcitx_input_context_new(server->xim->icManager, fcitx_xim_frontend.frontendId, NULL, NULL);
            FcitxXIMICData* icData = fcitx_utils_new(FcitxXIMICData);
            icData->ic = ic;
            icData->server = server;
            xcb_im_input_context_set_data(xic, icData, free);
            fcitx_input_context_set_property(ic, server->xim->frontendDataId, xic);
            fcitx_input_context_set_focus_group(ic, FICFG_Local, server->group);
            break;
        case XCB_XIM_DESTROY_IC:
            fcitx_input_context_destroy(ic);
            break;
        case XCB_XIM_SET_IC_VALUES:
            // kinds of like notification for position moving
            break;
        case XCB_XIM_FORWARD_EVENT:
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
            event.time = xevent->time;
            if (!fcitx_input_context_process_key_event(ic, &event)) {
                xcb_im_forward_event(im, xic, xevent);
            }
            break;
        }
        case XCB_XIM_RESET_IC:
            fcitx_input_context_reset(ic);
            break;
        case XCB_XIM_SET_IC_FOCUS:
            fcitx_input_context_focus_in(ic);
            break;
        case XCB_XIM_UNSET_IC_FOCUS:
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
    server->root = screen->root;
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
    free(server->name);
    free(server);
}

static const UT_icd ut_feedback_icd _FCITX_UNUSED_ = {
    sizeof(uint32_t), NULL, NULL, NULL
};

void* fcitx_xim_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxXIM* xim = fcitx_utils_new(FcitxXIM);
    FcitxInputContextManager* icmanager = fcitx_addon_manager_get_property(manager, "icmanager");

    xcb_compound_text_init();

    xim->icManager = icmanager;
    xim->frontendDataId = fcitx_input_context_manager_lookup_property(icmanager, FCITX_FRONTEND_DATA_PROPERTY);
    xim->manager = manager;
    xim->connFilterId = fcitx_dict_new(fcitx_xim_server_destroy);

    xim->connCreatedId = fcitx_xcb_invoke_on_connection_created(manager, fcitx_xim_on_xcb_connection_created, xim);
    xim->connClosedId =fcitx_xcb_invoke_on_connection_closed(manager, fcitx_xim_on_xcb_connection_closed, xim);
    utarray_init(&xim->feedbackBuffer, &ut_feedback_icd);

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

bool fcitx_xim_handle_event(void* data, FcitxEvent* _event)
{
    FcitxXIM* xim = data;
    FcitxInputContextEvent* event = (FcitxInputContextEvent*) _event;
    if ((event->type & ET_EventTypeFlag) != ET_InputMethodEventFlag) {
        return false;
    }
    xcb_im_input_context_t* xic = fcitx_input_context_get_property(event->inputContext, xim->frontendDataId);
    FcitxXIMICData* icData = xcb_im_input_context_get_data(xic);
    FcitxXIMServer* server = icData->server;

    switch (event->type) {
        case ET_InputContextForwardKey: {
                FcitxInputContextKeyEvent* keyEvent = (FcitxInputContextKeyEvent*) event;
                xcb_key_press_event_t xcbEvent;
                memset(&xcbEvent, 0, sizeof(xcb_key_press_event_t));
                xcbEvent.time = keyEvent->detail.time;
                xcbEvent.response_type = keyEvent->detail.isRelease ? XCB_KEY_PRESS : XCB_KEY_RELEASE;
                xcbEvent.state = keyEvent->detail.rawKey.state;
                xcbEvent.detail = keyEvent->detail.keyCode;
                xcbEvent.root = server->root;
                xcbEvent.event = xcb_im_input_context_get_focus_window(xic);
                if ((xcbEvent.event = xcb_im_input_context_get_focus_window(xic)) == XCB_WINDOW_NONE) {
                    xcbEvent.event = xcb_im_input_context_get_client_window(xic);
                }
                xcbEvent.child = XCB_WINDOW_NONE;
                xcbEvent.same_screen = 0;
                xcbEvent.sequence = 0;
                xcb_im_forward_event(server->im, xic, &xcbEvent);
            }
            break;
        case ET_InputContextCommitString: {
                FcitxInputContextCommitStringEvent* commitStringEvent = (FcitxInputContextCommitStringEvent*) event;
                xcb_im_commit_string(server->im, xic, XCB_XIM_LOOKUP_CHARS, commitStringEvent->commitString, strlen(commitStringEvent->commitString), 0);
            }
            break;
        case ET_InputContextDeleteSurroundingText:
            break;
        case ET_InputContextUpdatePreedit: {
                FcitxText* preedit = fcitx_input_context_get_preedit(event->inputContext);
                char* strPreedit = fcitx_text_to_string(preedit);
                // TODO FILTER

                size_t preeditLength = strlen(strPreedit);
                if (preeditLength == 0 && icData->preeditStarted) {
                    xcb_im_preedit_draw_fr_t frame;
                    memset(&frame, 0, sizeof(xcb_im_preedit_draw_fr_t));
                    frame.caret = 0;// TODO caret
                    frame.chg_first = 0;
                    frame.chg_length = icData->lastPreeditLength;
                    frame.length_of_preedit_string = 0;
                    frame.preedit_string =  NULL;
                    frame.feedback_array.size = 0;
                    frame.feedback_array.items = NULL;
                    frame.status = 1;
                    xcb_im_preedit_draw_callback(server->im, xic, &frame);
                    xcb_im_preedit_done_callback(server->im, xic);
                    icData->preeditStarted = false;
                }

                if (preeditLength != 0 && !icData->preeditStarted) {
                    xcb_im_preedit_start(server->im, xic);
                    icData->preeditStarted = true;
                }
                if (preeditLength != 0) {
                    size_t utf8Length = fcitx_utf8_strlen(strPreedit);
                    utarray_clear(&server->xim->feedbackBuffer);

                    for (size_t i = 0, offset = 0; i < fcitx_text_size(preedit); i++) {
                        FcitxTextFormatFlags format = fcitx_text_get_format(preedit, i);
                        const char* str = fcitx_text_get_string(preedit, i);
                        uint32_t feedback = 0;
                        if (format & FTFF_UnderLine) {
                            feedback |= XCB_XIM_UNDERLINE;
                        }
                        if (format & FTFF_HighLight) {
                            feedback |= XCB_XIM_REVERSE;
                        }
                        unsigned int strLen = fcitx_utf8_strlen(str);
                        for (size_t j = 0;j < strLen;j++) {
                            utarray_push_back(&server->xim->feedbackBuffer, &feedback);
                            offset++;
                        }
                    }

                    while (utarray_len(&server->xim->feedbackBuffer) > 0 && *((uint32_t*) utarray_back(&server->xim->feedbackBuffer)) == 0) {
                        utarray_pop_back(&server->xim->feedbackBuffer);
                    }

                    xcb_im_preedit_draw_fr_t frame;
                    memset(&frame, 0, sizeof(xcb_im_preedit_draw_fr_t));
                    frame.caret = 0;// TODO caret
                    frame.chg_first = 0;
                    frame.chg_length = icData->lastPreeditLength;
                    size_t compoundTextLength;
                    char* compoundText = xcb_utf8_to_compound_text(strPreedit, preeditLength, &compoundTextLength);
                    if (!compoundText) {
                        return true;
                    }
                    frame.length_of_preedit_string = compoundTextLength;
                    frame.preedit_string = (uint8_t*) compoundText;
                    frame.feedback_array.size = utarray_len(&server->xim->feedbackBuffer);
                    frame.feedback_array.items = server->xim->feedbackBuffer.d;
                    frame.status = frame.feedback_array.size ? 0 : 2;
                    icData->lastPreeditLength = utf8Length;
                    xcb_im_preedit_draw_callback(server->im, xic, &frame);
                }

                free(strPreedit);
            }
            break;
        default:
            break;
    }
    return true;
}
