#include <xcb/xcb.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "fcitx-xcb-internal.h"

typedef struct _FcitxXCB
{

    FcitxInstance* instance;
    FcitxDict* conns;
} FcitxXCB;

typedef struct _FcitxXCBConnection
{
    xcb_connection_t* conn;
    int screen;
    FcitxIOEvent* event;
    FcitxXCB* xcb;
    char* name;
} FcitxXCBConnection;

static void* fcitx_xcb_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_xcb_destroy(void* data);

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
        // TODO run the dispatcher

        free(event);
    }

}

void fcitx_xcb_connection_close(void* data)
{
    FcitxXCBConnection* fconn = data;
    FcitxXCB* xcb = fconn->xcb;
    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(xcb->instance);
    fcitx_mainloop_remove_io_event(mainloop, fconn->event);
    xcb_disconnect(fconn->conn);

    free(fconn);
}

void* fcitx_xcb_init(FcitxAddonManager* manager, const FcitxAddonConfig* config)
{
    FCITX_UNUSED(config);
    FcitxXCB* xcb = fcitx_utils_new(FcitxXCB);
    FcitxInstance* instance = fcitx_addon_manager_get_property(manager, "instance");
    xcb->instance = instance;
    xcb->conns = fcitx_dict_new(fcitx_xcb_connection_close);

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
        xcb_disconnect(conn);
        return;
    }

    FcitxXCBConnection* fconn = fcitx_utils_new(FcitxXCBConnection);
    fconn->xcb = xcb;
    fconn->conn = conn;
    fconn->name = strdup(name);
    int fd = xcb_get_file_descriptor(conn);
    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(xcb->instance);
    fconn->event = fcitx_mainloop_register_io_event(mainloop, fd, FIOEF_IN, fcitx_xcb_io_callback, NULL, fconn);

    fcitx_dict_insert_by_str(xcb->conns, name, fconn, false);
}

void fcitx_xcb_destroy(void* data)
{
    FcitxXCB* xcb = data;
    fcitx_dict_free(xcb->conns);
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

void fcitx_xcb_on_connection_created(FcitxXCB* self, void* arg0)
{
}

void fcitx_xcb_on_connection_closed(FcitxXCB* self, void* arg0)
{
}
