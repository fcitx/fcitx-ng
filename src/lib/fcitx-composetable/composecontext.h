/*
 * Copyright (C) 2014~2015 by CSSlayer
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

#ifndef FCITX_COMPSOTETABLE_COMPOSECONTEXT_H
#define FCITX_COMPSOTETABLE_COMPOSECONTEXT_H

#include <fcitx-utils/utils.h>
#include "tablegenerator.h"

typedef struct _FcitxComposeContext FcitxComposeContext;

FcitxComposeContext* fcitx_compose_context_new(FcitxComposeTable* table);
FcitxComposeContext* fcitx_compose_context_ref(FcitxComposeContext* context);
void fcitx_compose_context_unref(FcitxComposeContext* context);
bool fcitx_compose_context_process_key(FcitxComposeContext* context, FcitxKey key);
uint32_t fcitx_compose_context_get_char(FcitxComposeContext* context);
const char* fcitx_compose_context_get_text(FcitxComposeContext* context);
void fcitx_compose_context_reset(FcitxComposeContext* context);

#endif // FCITX_COMPSOTETABLE_COMPOSECONTEXT_H
