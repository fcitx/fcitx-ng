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

#include <pthread.h>
#include "atomic.h"

#ifdef __FCITX_ATOMIC_USE_SYNC_FETCH
/**
 * Also define lib function when there is builtin function for
 * atomic operation in case the function address is needed or the builtin
 * is not available when compiling other modules.
 **/
#define FCITX_UTIL_DEFINE_ATOMIC(name, op, type)                        \
    FCITX_EXPORT_API type                                               \
    (fcitx_utils_atomic_##name)(volatile type *atomic, type val)        \
    {                                                                   \
        return __sync_fetch_and_##name(atomic, val);                    \
    }
#else
static pthread_mutex_t __fcitx_utils_atomic_lock = PTHREAD_MUTEX_INITIALIZER;
#define FCITX_UTIL_DEFINE_ATOMIC(name, op, type)                        \
    FCITX_EXPORT_API type                                               \
    (fcitx_utils_atomic_##name)(volatile type *atomic, type val)        \
    {                                                                   \
        type oldval;                                                    \
        pthread_mutex_lock(&__fcitx_utils_atomic_lock);                 \
        oldval = *atomic;                                               \
        *atomic = oldval op val;                                        \
        pthread_mutex_unlock(&__fcitx_utils_atomic_lock);               \
        return oldval;                                                  \
    }
#endif

FCITX_UTIL_DEFINE_ATOMIC(add, +, int32_t)
FCITX_UTIL_DEFINE_ATOMIC(and, &, uint32_t)
FCITX_UTIL_DEFINE_ATOMIC(or, |, uint32_t)
FCITX_UTIL_DEFINE_ATOMIC(xor, ^, uint32_t)

#undef FCITX_UTIL_DEFINE_ATOMIC
