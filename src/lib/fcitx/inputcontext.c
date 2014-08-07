#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"
#include "inputcontext-internal.h"

typedef struct _FcitxInputContextManager
{
    int refcount;
    uint32_t icid;
    FcitxInputContext* freeList;
    FcitxInputContext* ics;
    FcitxInputContext* currentIC;
} FcitxInputContextManager;

FcitxInputContextManager* fcitx_input_context_manager_new()
{
    FcitxInputContextManager* manager = fcitx_utils_new(FcitxInputContextManager);
    return fcitx_input_context_manager_ref(manager);
}

void fcitx_input_context_manager_free(FcitxInputContextManager* manager)
{
    free(manager);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxInputContextManager, fcitx_input_context_manager);

FcitxInputContext* fcitx_input_context_manager_create_ic(FcitxInputContextManager* manager,
                                                         FcitxInputContextFillDataCallback callback,
                                                         void* userData)
{
    FcitxInputContext* ic = NULL;
    if (manager->freeList) {
        ic = manager->freeList;
        manager->freeList = ic->hh.next;
    } else {
        uint32_t newid;
        while ((newid = ++manager->icid) != 0);
        HASH_FIND(hh, manager->ics, &newid, sizeof(uint32_t), ic);
        if (ic) {
            return NULL;
        }
        ic = fcitx_utils_new(FcitxInputContext);
        ic->id = newid;
    }

    if (callback) {
        callback(ic, userData);
    }

    return ic;
}

void fcitx_input_context_manager_focus_in(FcitxInputContextManager* manager, uint32_t id)
{
    FcitxInputContext* ic = NULL;
    HASH_FIND(hh, manager->ics, &id, sizeof(uint32_t), ic);
    if (ic) {
        manager->currentIC = ic;
    }
}

void fcitx_input_context_manager_focus_out(FcitxInputContextManager* manager, uint32_t id)
{
    if (manager->currentIC && id && manager->currentIC->id == id) {
        manager->currentIC = NULL;
    }
}

FcitxInputContext* fcitx_input_context_manager_get_ic(FcitxInputContextManager* manager, uint32_t id)
{
    FcitxInputContext* ic = NULL;
    HASH_FIND(hh, manager->ics, &id, sizeof(uint32_t), ic);
    return ic;
}

