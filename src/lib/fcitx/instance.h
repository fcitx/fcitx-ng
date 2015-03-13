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


/**
 * @addtogroup Fcitx
 * @{
 */

#ifndef __FCITX_INSTANCE_H__
#define __FCITX_INSTANCE_H__

#include <fcitx-utils/utils.h>
#include <fcitx/inputcontext.h>

FCITX_DECL_BEGIN

typedef struct _FcitxInstance FcitxInstance;

FcitxInstance* fcitx_instance_create(int argc, char* argv[]);
int fcitx_instance_run(FcitxInstance* instance);
FcitxMainLoop* fcitx_instance_get_mainloop(FcitxInstance* instance);
bool fcitx_instance_get_try_replace(FcitxInstance* instance);
void fcitx_instance_set_signal_pipe(FcitxInstance* instance, int fd);
void fcitx_instance_shutdown(FcitxInstance* instance);
void fcitx_instance_destroy(FcitxInstance* instance);

FCITX_DECL_END

#endif
/**
 * @}
 */
