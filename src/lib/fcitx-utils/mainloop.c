#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include "utils.h"

static UT_icd pollfd_icd = {
    sizeof(struct pollfd), NULL, NULL, NULL
};

typedef struct _FcitxPollKey
{
    int fd;
    int flag;
} FcitxPollKey;

struct _FcitxIOEvent
{
    FcitxIOEventCallback callback;
    int fd;
    FcitxListHead list;
    FcitxDestroyNotify destroyNotify;
    void* userdata;
    int id;
};

struct _FcitxTimeoutEvent
{
    FcitxTimeoutEventCallback callback;
    uint32_t timeout;
    FcitxListHead list;
    bool repeat;
    int64_t time;
    void* userdata;
    FcitxDestroyNotify destroyNotify;
};

struct _FcitxMainLoop
{
    FcitxHandlerTable* ioHandler;
    FcitxListHead timeoutHandler;
    int64_t nextTimeout;

    bool rebuildPollfds;
    UT_array pollfds;
    int npollfds;

    bool quit;
    int wakeupPipe[2];
    int pollRet;
};

static void fcitx_mainloop_wakeup(FcitxMainLoop* mainloop);
static bool get_time(int64_t* ms);
static int fcitx_io_event_flag_to_libc_flag(int flag)
{
    return ((flag & FIOEF_IN) ? POLLIN : 0)
         | ((flag & FIOEF_OUT) ? POLLOUT : 0)
         | ((flag & FIOEF_ERR) ? POLLERR : 0)
         | ((flag & FIOEF_HUP) ? POLLHUP : 0);
}
static int fcitx_io_event_flag_from_libc_flag(int flag)
{
    return ((flag & POLLIN) ? FIOEF_IN : 0)
         | ((flag & POLLOUT) ? FIOEF_OUT : 0)
         | ((flag & POLLERR) ? FIOEF_ERR : 0)
         | ((flag & POLLHUP) ? FIOEF_HUP : 0);
}

void fd_rebuild_notify(void *data, const void* key, size_t len, void *owner)
{
    FCITX_UNUSED(data);
    FCITX_UNUSED(key);
    FCITX_UNUSED(len);
    FcitxMainLoop* mainloop = owner;
    mainloop->rebuildPollfds = true;
}

void ioevent_free(void* data)
{
    FcitxIOEvent* event = data;
    if (event->destroyNotify) {
        event->destroyNotify(event->userdata);
    }
}


static inline void
set_time_event(FcitxTimeoutEvent* event)
{
    if (event->timeout == 0) {
        event->time = 0;
    } else {
        get_time(&event->time);
        event->time += event->timeout;
    }
}

FCITX_EXPORT_API
FcitxMainLoop* fcitx_mainloop_new(void )
{
    int64_t t;
    if (!get_time(&t)) {
        return NULL;
    }

    FcitxMainLoop* mainloop = fcitx_utils_new(FcitxMainLoop);
    if (!mainloop) {
        return NULL;
    }

    if (pipe(mainloop->wakeupPipe) < 0) {
        goto fcitx_mainloop_new_error_end;
    }

    if (fcntl(mainloop->wakeupPipe[0], F_SETFD, FD_CLOEXEC) == -1
     || fcntl(mainloop->wakeupPipe[0], F_SETFL, O_NONBLOCK) == -1
     || fcntl(mainloop->wakeupPipe[1], F_SETFD, FD_CLOEXEC) == -1
     || fcntl(mainloop->wakeupPipe[1], F_SETFL, O_NONBLOCK) == -1) {
        goto fcitx_mainloop_new_error_end;
    }

    utarray_init(&mainloop->pollfds, &pollfd_icd);

    FcitxHandlerKeyDataVTable vtable;
    vtable.owner = mainloop;
    vtable.size = 0;
    vtable.init = fd_rebuild_notify;
    vtable.free = fd_rebuild_notify;
    mainloop->ioHandler = fcitx_handler_table_new(sizeof(FcitxIOEvent), ioevent_free, &vtable);
    fcitx_list_init(&mainloop->timeoutHandler);
    mainloop->rebuildPollfds = true;

    return mainloop;

fcitx_mainloop_new_error_end:
    free(mainloop);
    return NULL;
}

void fcitx_mainloop_wakeup(FcitxMainLoop* mainloop)
{
    uint8_t b = 0;
    write(mainloop->wakeupPipe[1], &b, sizeof(b));
}

int fcitx_mainloop_poll(FcitxMainLoop* mainloop)
{
    mainloop->pollRet = 0;
    if (mainloop->quit) {
        return -1;
    }

    if (mainloop->nextTimeout == 0) {
        return 0;
    }

    struct timespec ts, *pts = NULL;
    if (mainloop->nextTimeout > 0) {
        ts.tv_sec = mainloop->nextTimeout / 1000LL;
        ts.tv_nsec = (mainloop->nextTimeout % 1000LL) * 1000000LL;
        pts = &ts;
    }
    mainloop->pollRet = ppoll((struct pollfd*) mainloop->pollfds.d, utarray_len(&mainloop->pollfds), pts, NULL);

    if (mainloop->pollRet < 0) {
        if (errno == EINTR) {
            mainloop->pollRet = 0;
        }
    }

    return mainloop->pollRet;
}

void fcitx_mainloop_dispatch_timeout(FcitxMainLoop* mainloop)
{
    int64_t now = 0;
    get_time(&now);
    fcitx_list_entry_foreach_safe(event, FcitxTimeoutEvent, &mainloop->timeoutHandler, list) {
        if (mainloop->quit) {
            break;
        }

        if (event->time < now) {
            event->callback(event, event->userdata);
            if (event->repeat) {
                set_time_event(event);
            } else {
                fcitx_mainloop_remove_timeout_event(mainloop, event);
            }
        }
    }
}

void fcitx_mainloop_dispatch_io(FcitxMainLoop* mainloop)
{
    if (mainloop->pollRet <= 0) {
        return;
    }

    int n = mainloop->pollRet;
    for (size_t i = 0; i < utarray_len(&mainloop->pollfds); i++) {
        if (mainloop->quit) {
            break;
        }
        if (n <= 0) {
            break;
        }

        struct pollfd* pfd = (struct pollfd*) utarray_eltptr(&mainloop->pollfds, i);
        if (pfd->revents) {
            n --;

            FcitxPollKey key;
            memset(&key, 0, sizeof(FcitxPollKey));
            key.fd = pfd->fd;
            key.flag = pfd->events;
            for (FcitxIOEvent* event = fcitx_handler_table_first(mainloop->ioHandler, sizeof(FcitxPollKey), &key);
                event;
                event = fcitx_handler_table_next(mainloop->ioHandler, event)) {
                event->callback(event, event->fd, fcitx_io_event_flag_from_libc_flag(pfd->revents), event->userdata);
                if (mainloop->quit) {
                    break;
                }
            }
            pfd->revents = 0;
        }
    }
}

bool get_time(int64_t* ms)
{
    int r = -1;
#ifdef HAVE_CLOCK_GETTIME
    struct timespec ts;
    do {
#ifdef CLOCK_MONOTONIC_RAW
        r = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        if (r == 0) {
            break;
        }
#endif
#ifdef CLOCK_MONOTONIC
        r = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (r == 0) {
            break;
        }
#endif
#ifdef CLOCK_REALTIME
        r = clock_gettime(CLOCK_REALTIME, &ts);
        if (r == 0) {
            break;
        }
#endif
    } while (0);
    if (r == 0) {
        *ms = ts.tv_sec * 1000LL + ts.tv_nsec / (1000000LL);
        return true;
    }
#endif
    struct timeval tv;
    r = gettimeofday(&tv, NULL);
    if (r == 0) {
        *ms = tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
    }
    return r == 0;
}

void foreach_poll_key(FcitxHandlerTable* table, FcitxHandlerKey* key, void* data)
{
    FcitxMainLoop* mainloop = data;
    utarray_extend_back(&mainloop->pollfds);
    struct pollfd* pfd = (struct pollfd*) utarray_back(&mainloop->pollfds);
    FcitxPollKey* pollkey = fcitx_handler_key_get_data(table, key);
    pfd->fd = pollkey->fd;
    pfd->events = pollkey->flag;
    pfd->revents = 0;
}

int fcitx_mainloop_prepare(FcitxMainLoop* mainloop)
{
    if (mainloop->quit) {
        return -1;
    }
    int64_t now = 0;
    get_time(&now);
    mainloop->nextTimeout = -1;

    char wakeUpBuf[16];
    while (read(mainloop->wakeupPipe[0], wakeUpBuf, sizeof(wakeUpBuf)) == sizeof(wakeUpBuf));

    fcitx_list_entry_foreach(event, FcitxTimeoutEvent, &mainloop->timeoutHandler, list) {
        if (event->time <= now) {
            mainloop->nextTimeout = 0;
            break;
        }

        if (event->time == 0) {
            mainloop->nextTimeout = 0;
            break;
        }

        if (mainloop->nextTimeout == -1 || event->time - now < mainloop->nextTimeout) {
            mainloop->nextTimeout = event->time - now;
        }
    }

    if (mainloop->rebuildPollfds) {
        utarray_clear(&mainloop->pollfds);
        utarray_extend_back(&mainloop->pollfds);
        struct pollfd* fd = (struct pollfd*) utarray_back(&mainloop->pollfds);
        fd->fd = mainloop->wakeupPipe[0];
        fd->events = POLLIN;
        fd->revents = 0;
        fcitx_handler_table_foreach_key(mainloop->ioHandler, foreach_poll_key, mainloop);
    }
    return 0;
}

int fcitx_mainloop_dispatch(FcitxMainLoop* mainloop)
{
    if (mainloop->quit) {
        return -1;
    }

    fcitx_mainloop_dispatch_timeout(mainloop);
    fcitx_mainloop_dispatch_io(mainloop);

    return 0;
}

int fcitx_mainloop_iterate(FcitxMainLoop* mainloop)
{
    int r = 0;
    do {
        if ((r = fcitx_mainloop_prepare(mainloop)) < 0) {
        }

        if ((r = fcitx_mainloop_poll(mainloop)) < 0) {
            break;
        }

        if ((r = fcitx_mainloop_dispatch(mainloop)) < 0) {
            break;
        }
    } while (0);

    return r;
}

FCITX_EXPORT_API
int fcitx_mainloop_run(FcitxMainLoop* mainloop)
{
    mainloop->quit = false;
    int r;
    while ((r = fcitx_mainloop_iterate(mainloop)) >= 0) {
    }
    return r;
}

FCITX_EXPORT_API
void fcitx_mainloop_quit(FcitxMainLoop* mainloop)
{
    mainloop->quit = true;
}

FCITX_EXPORT_API
FcitxIOEvent* fcitx_mainloop_register_io_event(FcitxMainLoop* mainloop, int fd, unsigned int flag, FcitxIOEventCallback callback, FcitxDestroyNotify freeFunc, void* userdata)
{
    FcitxIOEvent event, *pevent;
    event.fd = fd;
    event.destroyNotify = freeFunc;
    event.userdata = userdata;
    event.callback = callback;

    FcitxPollKey key;
    memset(&key, 0, sizeof(FcitxPollKey));
    key.fd = fd;
    key.flag = fcitx_io_event_flag_to_libc_flag(flag);
    int id = fcitx_handler_table_append(mainloop->ioHandler, sizeof(FcitxPollKey), &key, &event);
    pevent = fcitx_handler_table_get_by_id(mainloop->ioHandler, id);
    pevent->id = id;

    fcitx_mainloop_wakeup(mainloop);

    return pevent;
}

FCITX_EXPORT_API
FcitxTimeoutEvent* fcitx_mainloop_register_timeout_event(FcitxMainLoop* mainloop, uint32_t timeout, bool repeat, FcitxTimeoutEventCallback callback, FcitxDestroyNotify freeFunc, void* userdata)
{
    FcitxTimeoutEvent* event = fcitx_utils_new(FcitxTimeoutEvent);
    event->timeout = timeout;
    set_time_event(event);
    event->repeat = repeat;
    event->destroyNotify = freeFunc;
    event->userdata = userdata;
    event->callback = callback;
    fcitx_list_prepend(&event->list, &mainloop->timeoutHandler);

    fcitx_mainloop_wakeup(mainloop);

    return event;
}

FCITX_EXPORT_API
void fcitx_mainloop_remove_io_event(FcitxMainLoop* mainloop, FcitxIOEvent* event)
{
    fcitx_handler_table_remove_by_id_full(mainloop->ioHandler, event->id);
}

FCITX_EXPORT_API
void fcitx_mainloop_remove_timeout_event(FcitxMainLoop* mainloop, FcitxTimeoutEvent* event)
{
    FCITX_UNUSED(mainloop);
    fcitx_list_remove(&event->list);
    if (event->destroyNotify) {
        event->destroyNotify(event->userdata);
    }
    free(event);
}

FCITX_EXPORT_API
void fcitx_mainloop_free(FcitxMainLoop* mainloop)
{
    fcitx_handler_table_free(mainloop->ioHandler);

    fcitx_list_entry_foreach_safe(event, FcitxTimeoutEvent, &mainloop->timeoutHandler, list) {
        fcitx_mainloop_remove_timeout_event(mainloop, event);
    }

    close(mainloop->wakeupPipe[0]);
    close(mainloop->wakeupPipe[1]);
    utarray_done(&mainloop->pollfds);

    free(mainloop);
}
