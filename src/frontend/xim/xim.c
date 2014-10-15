#include <xcb-imdkit/imdkit.h>
#include <xcb/xcb_aux.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "module/xcb/fcitx-xcb.h"

typedef struct _FcitxXIM
{
    FcitxInstance* instance;
    FcitxAddonManager* manager;
    FcitxDict* connFilterId;
} FcitxXIM;

typedef struct _FcitxXIMServer
{
    xcb_window_t window;
    FcitxXIM* xim;
    xcb_im_t* im;
    int id;
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


static void* fcitx_xim_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_xim_destroy(void* data);

FCITX_DEFINE_ADDON(fcitx_xim, frontend, FcitxAddonAPICommon) = {
    .init = fcitx_xim_init,
    .destroy = fcitx_xim_destroy
};


bool callback(xcb_im_t* im, xcb_im_client_t* client, xcb_im_input_context_t* ic,
              const xcb_im_packet_header_fr_t* hdr,
              void* frame, void* arg, void* user_data)
{
    FCITX_UNUSED(im);
    FCITX_UNUSED(client);
    FCITX_UNUSED(ic);
    FCITX_UNUSED(hdr);
    FCITX_UNUSED(frame);
    FCITX_UNUSED(arg);
    FCITX_UNUSED(user_data);
    return false;
}

bool fcitx_xim_xcb_event_fitler(xcb_connection_t* conn, xcb_generic_event_t* event, void* data)
{
    FCITX_UNUSED(conn);
    FcitxXIMServer* server = data;
    return xcb_im_filter_event(server->im, event);
}

void fcitx_xim_on_xcb_connection_created(const char* name, xcb_connection_t* conn, int defaultScreen, void* data)
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
    server->im = xcb_im_create(conn,
                               defaultScreen,
                               w,
                               "fcitx",
                               XCB_IM_ALL_LOCALES,
                               &styles,
                               NULL,
                               NULL,
                               &encodings,
                               0,
                               callback,
                               server);
    server->id = fcitx_xcb_invoke_add_event_filter(self->manager, (void*) name, fcitx_xim_xcb_event_fitler, server);
    fcitx_dict_insert_by_str(self->connFilterId, name, server, false);

    xcb_im_open_im(server->im);
}

void fcitx_xim_on_xcb_connection_closed(const char* name, xcb_connection_t* conn, void* data)
{
    FCITX_UNUSED(conn);
    FcitxXIM* self = data;
    FcitxXIMServer* server = NULL;
    if (fcitx_dict_lookup_by_str(self->connFilterId, name, &server)) {
        return;
    }
    fcitx_xcb_invoke_remove_watcher(self->manager, server->id);
    xcb_im_destroy(server->im);
    // connection is going to close, what's the point of destroy window?
    free(server);
}

void* fcitx_xim_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxXIM* xim = fcitx_utils_new(FcitxXIM);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    xim->instance = instance;
    xim->manager = manager;
    xim->connFilterId = fcitx_dict_new(NULL);

    fcitx_xcb_invoke_on_connection_created(manager, fcitx_xim_on_xcb_connection_created, xim);
    fcitx_xcb_invoke_on_connection_closed(manager, fcitx_xim_on_xcb_connection_closed, xim);

    return xim;
}

void fcitx_xim_destroy(void* data)
{
    FcitxXIM* xim = data;
    free(xim);
}
