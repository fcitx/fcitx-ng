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

#ifndef __FCITX_ADDON_INTERNAL_H__
#define __FCITX_ADDON_INTERNAL_H__

#include <stdint.h>
#include "addon.h"
#include "addon-config.h"

/**
 * Addon Instance in Fcitx
 **/
struct _FcitxAddon {
    FcitxAddonConfig *config;
    bool loaded;
    FcitxStringHashSet* dependencies;
    FcitxAddonInstance inst;
};

struct _FcitxAddonMananger {
    FcitxDict* resolvers;
    FcitxStandardPath* standardPath;
    int32_t refcount;
    FcitxDict* addons;

    // override data;
    FcitxStringHashSet* enabledAddons;
    FcitxStringHashSet* disabledAddons;
    bool disabledAllAddons;

    // load order;
    FcitxPtrArray* loadedAddons;
    bool loaded;
    FcitxDict* properties;

    FcitxPtrArray* frontends;
};

#endif // __FCITX_ADDON_INTERNAL_H__
