#ifndef _FCITX_INPUTCONTEXT_INTERNAL_H_
#define _FCITX_INPUTCONTEXT_INTERNAL_H_

#include <uuid/uuid.h>
#include "inputcontext.h"
#include "fcitx-utils/uthash.h"

/**
 * Input Context, normally one for one program
 **/
struct _FcitxInputContext {
    uint32_t id;
    FcitxCapabilityFlags flags; /**< input context capacity */
    char* imname;
    bool switchBySwitchKey;
    UT_array* data;
    char* prgname; /**< program name */
    FcitxTriState mayUsePreedit;
    UT_hash_handle hh;
    FcitxInputContextManager* manager;
    FcitxDestroyNotify destroyNotify;
    uuid_t uuid;
    FcitxInputContextGroup* group;

    FcitxListHead list;
    bool focused;
};

#endif // _FCITX_INPUTCONTEXT_INTERNAL_H_
