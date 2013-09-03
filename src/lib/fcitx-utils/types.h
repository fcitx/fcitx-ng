#ifndef _FCITX_UTILS_TYPES_H_
#define _FCITX_UTILS_TYPES_H_

#include <fcitx-utils/macro.h>
#include <stdint.h>

FCITX_DECL_BEGIN

/**
 * fcitx boolean
 **/
typedef int32_t boolean;
#if !defined(__cplusplus) && !defined(FCITX_DONOT_DEFINE_TRUE_FALSE)
/**
 * fcitx true
 */
#define true (1)
/**
 * fcitx false
 */
#define false (0)
#endif

FCITX_DECL_END

#endif
