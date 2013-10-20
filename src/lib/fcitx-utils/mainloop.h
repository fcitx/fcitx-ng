#include <fcitx-utils/types.h>

typedef struct _FcitxMainLoop FcitxMainLoop;
typedef struct _FcitxIOEvent FcitxIOEvent;
typedef struct _FcitxTimeoutEvent FcitxTimeoutEvent;

typedef void (*FcitxIOEventCallback)(FcitxIOEvent* event, int fd, int flag, void* data);
typedef void (*FcitxTimeoutEventCallback)(FcitxTimeoutEvent* event, void* data);

typedef enum _FcitxIOEventFlag
{
    FIOEF_IN = (1 << 0),
    FIOEF_OUT = (1 << 1),
    FIOEF_ERR = (1 << 2),
    FIOEF_HUP = (1 << 3),
} FcitxIOEventFlag;

FcitxMainLoop* fcitx_mainloop_new(void);
int fcitx_mainloop_run(FcitxMainLoop* mainloop);
void fcitx_mainloop_quit(FcitxMainLoop* mainloop);
FcitxIOEvent* fcitx_mainloop_register_io_event(FcitxMainLoop* mainloop, int fd, int flag, FcitxIOEventCallback callback, FcitxDestroyNotify freeFunc, void* userdata);
FcitxTimeoutEvent* fcitx_mainloop_register_timeout_event(FcitxMainLoop* mainloop, uint32_t timeout, boolean repeat, FcitxTimeoutEventCallback callback, FcitxDestroyNotify freeFunc, void* userdata);
void fcitx_mainloop_remove_io_event(FcitxMainLoop* mainloop, FcitxIOEvent* event);
void fcitx_mainloop_remove_timeout_event(FcitxMainLoop* mainloop, FcitxTimeoutEvent* event);
void fcitx_mainloop_free(FcitxMainLoop* mainloop);
