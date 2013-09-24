#ifndef _FCITX_INPUTCONTEXT_INTERNAL_H_
#define _FCITX_INPUTCONTEXT_INTERNAL_H_

#include "inputcontext.h"
#include <fcitx-utils/types.h>
#include <fcitx-utils/utarray.h>

/**
 * Input Context, normally one for one program
 **/
struct _FcitxInputContext {
    FcitxInputContextState state; /**< input method state */
    FcitxCapacityFlags contextCaps; /**< input context capacity */
    int offsetX; /**< x offset to the window */
    int offsetY; /**< y offset to the window */
    char* imname;
    boolean switchBySwitchKey;
    UT_array* data;
    char* prgname; /**< program name */
    FcitxTriState mayUsePreedit;
};

#endif // _FCITX_INPUTCONTEXT_INTERNAL_H_
