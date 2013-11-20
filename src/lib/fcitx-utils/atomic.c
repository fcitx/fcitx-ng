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