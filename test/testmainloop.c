#include <fcitx-utils/mainloop.h>
#include <stdlib.h>

void callback(FcitxTimeoutEvent* event, void* data)
{
    FcitxMainLoop* mainloop = data;
    fcitx_mainloop_quit(mainloop);
}

int main()
{
    FcitxMainLoop* mainloop = fcitx_mainloop_new();
    fcitx_mainloop_register_timeout_event(mainloop, 1000, false, callback, NULL, mainloop);
    fcitx_mainloop_run(mainloop);

    return 0;
}
