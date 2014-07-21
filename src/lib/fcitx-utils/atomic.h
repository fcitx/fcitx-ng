#ifndef FCITX_UTILS_AUTOMIC_H
#define FCITX_UTILS_AUTOMIC_H

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
#define __FCITX_ATOMIC_USE_SYNC_FETCH
#elif defined __has_builtin
#if __has_builtin(__sync_fetch_and_add) &&      \
    __has_builtin(__sync_fetch_and_and) &&      \
    __has_builtin(__sync_fetch_and_xor) &&      \
    __has_builtin(__sync_fetch_and_or)
#define __FCITX_ATOMIC_USE_SYNC_FETCH
#endif
#endif

#ifdef __FCITX_ATOMIC_USE_SYNC_FETCH
#define FCITX_UTIL_DECLARE_ATOMIC(name, type)                           \
    type (fcitx_utils_atomic_##name)(volatile type *atomic, type val);  \
    static inline type                                                  \
    __fcitx_utils_atomic_##name(volatile type *atomic, type val)        \
    {                                                                   \
        return __sync_fetch_and_##name(atomic, val);                    \
    }
#else
#define FCITX_UTIL_DECLARE_ATOMIC(name, type)                           \
    type (fcitx_utils_atomic_##name)(volatile type *atomic, type val);  \
    static inline type                                                  \
    __fcitx_utils_atomic_##name(volatile type *atomic, type val)        \
    {                                                                   \
        return (fcitx_utils_atomic_##name)(atomic, val);                \
    }
#endif

    FCITX_UTIL_DECLARE_ATOMIC(add, int32_t)
    FCITX_UTIL_DECLARE_ATOMIC(and, uint32_t)
    FCITX_UTIL_DECLARE_ATOMIC(or, uint32_t)
    FCITX_UTIL_DECLARE_ATOMIC(xor, uint32_t)

#define fcitx_utils_atomic_add(atomic, val)     \
    __fcitx_utils_atomic_add(atomic, val)
#define fcitx_utils_atomic_and(atomic, val)     \
    __fcitx_utils_atomic_and(atomic, val)
#define fcitx_utils_atomic_or(atomic, val)      \
    __fcitx_utils_atomic_or(atomic, val)
#define fcitx_utils_atomic_xor(atomic, val)     \
    __fcitx_utils_atomic_xor(atomic, val)

#undef FCITX_UTIL_DECLARE_ATOMIC


#endif // FCITX_UTILS_AUTOMIC_H
