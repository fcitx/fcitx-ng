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

#ifndef FCITX_CONFIG_CONFIGURATION_H
#define FCITX_CONFIG_CONFIGURATION_H

#include <fcitx-utils/utils.h>

typedef struct _FcitxConfiguration FcitxConfiguration;

typedef void (*FcitxConfigurationForeachCallback)(FcitxConfiguration* config,
                                                  const char* path, void* userData);

typedef int (*FcitxConfigurationCompareCallback)(const FcitxConfiguration* configA, const FcitxConfiguration* configB, void* userData);

FcitxConfiguration* fcitx_configuration_new(const char* name);

FcitxConfiguration* fcitx_configuration_get(FcitxConfiguration* config, const char* path, bool createPath);

void fcitx_configuration_remove(FcitxConfiguration* config, const char* path);

void fcitx_configuration_set_value(FcitxConfiguration* config, const char* value);

void fcitx_configuration_set_comment(FcitxConfiguration* config, const char* comment);

const char* fcitx_configuration_get_comment(FcitxConfiguration* config);

const char* fcitx_configuration_get_name(const FcitxConfiguration* config);

const char* fcitx_configuration_get_value(const FcitxConfiguration* config);

bool fcitx_configuration_has_sub_items(const FcitxConfiguration* config);

void fcitx_configuration_foreach(FcitxConfiguration* config, const char* path, bool recursive, const char* pathPrefix, FcitxConfigurationForeachCallback callback, void* userData);

void fcitx_configuration_sort(FcitxConfiguration* config, const char* path, FcitxConfigurationCompareCallback compare, void* userData);

FcitxConfiguration* fcitx_configuration_ref(FcitxConfiguration* config);

void fcitx_configuration_unref(FcitxConfiguration* config);

static inline
void fcitx_configuration_set_value_by_path(FcitxConfiguration* config, const char* path, const char* value) {
    config = fcitx_configuration_get(config, path, true);
    if (config) {
        fcitx_configuration_set_value(config, value);
    }
}

static inline
const char* fcitx_configuration_get_value_by_path(FcitxConfiguration* config, const char* path) {
    config = fcitx_configuration_get(config, path, false);
    if (config) {
        return fcitx_configuration_get_value(config);
    }
    return NULL;
}

static inline
void fcitx_configuration_set_comment_by_path(FcitxConfiguration* config, const char* path, const char* value) {
    config = fcitx_configuration_get(config, path, true);
    if (config) {
        fcitx_configuration_set_value(config, value);
    }
}

#endif // FCITX_CONFIG_CONFIGURATION_H
