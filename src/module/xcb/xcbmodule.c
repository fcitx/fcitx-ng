#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "fcitx/addon.h"
#include "fcitx/frontend.h"
#include "fcitx/instance.h"
#include "fcitx-xcb-internal.h"

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
    FcitxInputContextGroup* group;
} FcitxXCBConnection;

typedef struct _FcitxXCBCallbackClosure
{
    FcitxCallback callback;
    void* userData;
} FcitxXCBCallbackClosure;

typedef void (*FcitxXCBConnectionCreatedCallback)(const char* name,
                                                  xcb_connection_t* conn,
                                                  int screen,
                                                  FcitxInputContextGroup* group,
                                                  void* userData);
typedef void (*FcitxXCBConnectionClosedCallback)(const char* name, xcb_connection_t* conn, void* userData);
typedef bool (*FcitxXCBEventFilterCallback)(xcb_connection_t* conn, xcb_generic_event_t*, void* userData);

static void* fcitx_xcb_init(FcitxAddonManager* manager, const FcitxAddonConfig* config);
static void fcitx_xcb_destroy(void* data);
void fcitx_xcb_open_connection(FcitxXCB* xcb, const char* name);

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

    FcitxInputContextGroup* group = fconn->group;
    fconn->group = NULL;
    fcitx_input_context_group_free(group);

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
    fconn->name = strdup(name);
    fconn->atom = atom;
    fconn->server_window = w;
    int fd = xcb_get_file_descriptor(conn);
    FcitxMainLoop* mainloop = fcitx_instance_get_mainloop(xcb->instance);
    fconn->event = fcitx_mainloop_register_io_event(mainloop, fd, FIOEF_IN, fcitx_xcb_io_callback, NULL, fconn);

    fcitx_dict_insert_by_str(xcb->conns, name, fconn, false);

    // create a focus group for display server
    FcitxInputContextGroup* group = fcitx_input_context_manager_create_group(xcb->icManager);
    fconn->group = group;

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
