#include <fcitx-utils/mainloop.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>

int p[2];
void timeout_callback(FcitxTimeoutEvent* event, void* data)
{
    char c = 0;
    assert(write(p[1], &c, sizeof(char)) == 1);
}

void timeout_callback2(FcitxTimeoutEvent* event, void* data)
{
    FcitxMainLoop* mainloop = data;
    fcitx_mainloop_quit(mainloop);
}


void io_callback(FcitxIOEvent* event, int fd, int revents, void* data)
{
    static int counter = 0;
    counter ++;

    assert(counter == 1);
    FcitxMainLoop* mainloop = data;
    char c[2];
    assert(read(p[0], c, sizeof(c)) == 1);

    fcitx_mainloop_register_timeout_event(mainloop, 100, false, timeout_callback2, NULL, mainloop);
}

int main()
{
    FcitxMainLoop* mainloop = fcitx_mainloop_new();
    int r = pipe(p);
    assert(r == 0);
    assert(fcntl(p[0], F_SETFL, O_NONBLOCK) != -1);
    assert(fcntl(p[1], F_SETFL, O_NONBLOCK) != -1);
    fcitx_mainloop_register_timeout_event(mainloop, 0, false, timeout_callback, NULL, mainloop);
    fcitx_mainloop_register_io_event(mainloop, p[0], FIOEF_IN, io_callback, NULL, mainloop);
    fcitx_mainloop_run(mainloop);

    fcitx_mainloop_free(mainloop);

    return 0;
}
