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

#ifndef _FCITX_UTILS_MACRO_H_
#define _FCITX_UTILS_MACRO_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

/** export the symbol */
#define FCITX_EXPORT_API __attribute__ ((visibility("default")))

/** suppress the unused warning */
#define FCITX_UNUSED(x) (void)(x)

#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 301)
# define FCITX_DEPRECATED  __attribute__((deprecated))
#else /* not gcc >= 3.1 */
# define FCITX_DEPRECATED
#endif

#ifdef __GNUC__
#define _FCITX_UNUSED_ __attribute__ ((__unused__))
#else
#define _FCITX_UNUSED_
#endif

#ifdef __GNUC__
#define _FCITX_ALWAYS_INLINE_ inline __attribute__ ((always_inline))
#else
#define _FCITX_ALWAYS_INLINE_ inline
#endif


#ifdef __cplusplus
#define FCITX_DECL_BEGIN extern "C" {
#define FCITX_DECL_END }
#else
#define FCITX_DECL_BEGIN
#define FCITX_DECL_END
#endif

#define FCITX_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define FCITX_INT_LEN ((int)(sizeof(int) * 2.5) + 2)
#define FCITX_LONG_LEN ((int)(sizeof(long) * 2.5) + 2)
#define FCITX_INT32_LEN (22)
#define FCITX_INT64_LEN (42)

#define fcitx_container_of(ptr, type, member)           \
    ((type*)(((void*)(ptr)) - offsetof(type, member)))

#if (defined(__GNUC__) && (__GNUC__ > 2))
#  define fcitx_expect(exp, var) __builtin_expect(exp, var)
#else
#  define fcitx_expect(exp, var) (exp)
#endif

#define fcitx_likely(x) fcitx_expect(!!(x), 1)
#define fcitx_unlikely(x) fcitx_expect(!!(x), 0)

#define FCITX_DEFINE_SIMPLE_UT_ICD(type, name)                      \
    static const UT_icd __fcitx_##name##_icd = {              \
        sizeof(type), NULL, NULL, NULL                        \
    };
#define FCITX_DEFINE_AND_EXPORT_SIMPLE_UT_ICD(type, name)     \
    FCITX_DEFINE_SIMPLE_UT_ICD(type, name)                    \
    FCITX_EXPORT_API const UT_icd *const fcitx_##name##_icd = \
        &__fcitx_##name##_icd;

#define FCITX_GET_PRIVATE(p, TYPE) ((TYPE##Private*) (((char*) p) + sizeof(TYPE)))

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define FCITX_PRINTF(format_idx, arg_idx) __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#else
#define FCITX_PRINTF(format_idx, arg_idx)
#endif

#endif
