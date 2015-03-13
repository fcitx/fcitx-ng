/*
 * Copyright (C) 2015~2015 by CSSlayer
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

#ifndef _FCITX_INPUTCONTEXT_INTERNAL_H_
#define _FCITX_INPUTCONTEXT_INTERNAL_H_

#include <uuid/uuid.h>
#include "inputcontext.h"
#include "fcitx-utils/uthash.h"

/**
 * Input Context, normally one for one program
 **/
struct _FcitxInputContext {
    uint32_t id;
    FcitxCapabilityFlags flags; /**< input context capacity */
    char* imname;
    bool switchBySwitchKey;
    UT_array* data;
    char* prgname; /**< program name */
    FcitxTriState mayUsePreedit;
    UT_hash_handle hh;
    FcitxInputContextManager* manager;
    FcitxDestroyNotify destroyNotify;
    uuid_t uuid;
    FcitxInputContextFocusGroup* group;

    FcitxListHead list;
    bool focused;
};

#endif // _FCITX_INPUTCONTEXT_INTERNAL_H_
