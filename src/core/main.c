/*
 * Copyright (C) 2010~2015 by CSSlayer
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

#include <unistd.h>
#include <fcntl.h>
#include <locale.h>
#include <libintl.h>
#include "fcitx/instance.h"
#include "errorhandler.h"

FcitxInstance* instance = NULL;
int selfpipe[2];
char* crashlog = NULL;

int main(int argc, char* argv[])
{
    if (pipe(selfpipe)) {
        fprintf(stderr, "Could not create self-pipe.\n");
        exit(1);
    }

    SetMyExceptionHandler();

    fcntl(selfpipe[0], F_SETFL, O_NONBLOCK);
    fcntl(selfpipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(selfpipe[1], F_SETFL, O_NONBLOCK);
    fcntl(selfpipe[1], F_SETFD, FD_CLOEXEC);

    char* localedir = fcitx_utils_get_fcitx_path("localedir");
    setlocale(LC_ALL, "");
    bindtextdomain("fcitx", localedir);
    free(localedir);
    bind_textdomain_codeset("fcitx", "UTF-8");
    textdomain("fcitx");

    instance = fcitx_instance_create(argc, argv);
    fcitx_instance_set_signal_pipe(instance, selfpipe[0]);
    int result = fcitx_instance_run(instance);
    fcitx_instance_destroy(instance);

    return result;
}
// kate: indent-mode cstyle; space-indent on; indent-width 0;
