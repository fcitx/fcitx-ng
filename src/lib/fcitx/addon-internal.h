#ifndef __FCITX_ADDON_INTERNAL_H__
#define __FCITX_ADDON_INTERNAL_H__

#include "addon.h"
#include "fcitx-utils/utarray.h"
#include "fcitx-utils/dict.h"
#include <stdint.h>

/**
 * How addon get input method list
 **/
typedef enum _IMRegisterMethod {
    IMRM_SELF,
    IMRM_EXEC,
    IMRM_CONFIGFILE
} IMRegisterMethod;

/**
 * Addon Instance in Fcitx
 **/
struct _FcitxAddon {
    char *name; /**< addon name, used as a identifier */
    char *generalname; /**< addon name, translatable user visible string */
    char *comment; /**< longer desc translatable user visible string */
    boolean enabled; /**< enabled or not*/
    uint32_t category; /**< addon category */
    char *library; /**< library string */
    char *depend; /**< dependency string */
    int priority; /**< priority */
    char *subconfig; /**< used by ui for subconfig */
    UT_array functionList; /**< addon exposed function */

    IMRegisterMethod registerMethod; /**< the input method register method */
    char* registerArgument; /**< extra argument for register, unused for now */
    char* uifallback; /**< if's a user interface addon, the fallback UI addon name */
    boolean advance; /**< a hint for GUI */
    boolean loadLocal;
};

struct _FcitxAddonInstance
{
    FcitxAddon* addon;
};

struct _FcitxAddonMananger {
    FcitxDict* resolvers;
};

#endif // __FCITX_ADDON_INTERNAL_H__
