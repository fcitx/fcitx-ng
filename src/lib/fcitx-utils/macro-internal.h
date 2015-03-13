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

#ifndef _FCITX_UTILS_MACRO_INTERNAL_H
#define _FCITX_UTILS_MACRO_INTERNAL_H

#define FCITX_REFCOUNT_FUNCTION_DEFINE(TYPENAME, SLUGNAME) \
FCITX_EXPORT_API \
TYPENAME* SLUGNAME##_ref(TYPENAME* data) \
{ \
    fcitx_utils_atomic_add (&data->refcount, 1); \
    return data; \
} \
FCITX_EXPORT_API \
void SLUGNAME##_unref(TYPENAME* data) \
{ \
    if (!data) { \
        return; \
    } \
    int32_t oldvalue = fcitx_utils_atomic_add (&data->refcount, -1); \
    if (oldvalue == 1) { \
        SLUGNAME##_free(data); \
    } \
}

#endif // _FCITX_UTILS_MACRO_INTERNAL_H
