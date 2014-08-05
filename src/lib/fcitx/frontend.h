#ifndef __FCITX_FRONTEND_H__
#define __FCITX_FRONTEND_H__

#include "addon.h"
#include "inputcontext.h"

typedef struct _FcitxAddonAPIFrontend
{
    FcitxAddonAPICommon common;
    void (*CreateIC)(void* arg, FcitxInputContext*, void* priv); /**< frontend create input context callback */
    bool (*CheckIC)(void* arg, FcitxInputContext* arg1, void* arg2); /**< frontend check context with private value callback */
    void (*DestroyIC)(void* arg, FcitxInputContext *context); /**< frontend destroy input context callback */
    void (*EnableIM)(void* arg, FcitxInputContext* arg1); /**< frontend enable input method to client callback */
    void (*CloseIM)(void* arg, FcitxInputContext* arg1); /**< frontend close input method to client callback */
    void (*CommitString)(void* arg, FcitxInputContext* arg1, const char* arg2); /**< frontend commit string callback */
    void (*ForwardKey)(void* arg, FcitxInputContext* arg1, FcitxKeyEvent event); /**< frontend forward key callback */
    void (*SetWindowOffset)(void* arg, FcitxInputContext* ic, int x, int y); /**< frontend set window offset callback */
    void (*GetWindowRect)(void* arg, FcitxInputContext* ic, int* x, int* y, int* w, int* h); /**< frontend get window position callback */
    void (*UpdatePreedit)(void* arg, FcitxInputContext* ic); /**< frontend update preedit callback */
    void (*UpdateClientSideUI)(void* arg, FcitxInputContext* ic); /**< frontend update client side user interface callback */
    void (*ReloadConfig)(void* arg); /**< frontend reload config callback */
    bool (*CheckICFromSameApplication)(void* arg, FcitxInputContext* icToCheck, FcitxInputContext* ic); /**< frontend check input context from same application callback */
    pid_t (*GetPid)(void* arg, FcitxInputContext* arg1); /**< get pid for ic, zero for unknown */
    void (*DeleteSurroundingText)(void* addonInstance, FcitxInputContext* ic, int offset, unsigned int size);
    bool (*GetSurroundingPreedit)(void* addonInstance, FcitxInputContext* ic, char** str, unsigned int* cursor, unsigned int* anchor);
    void* padding1;
    void* padding2;
    void* padding3;
    void* padding4;
    void* padding5;
    void* padding6;
    void* padding7;
    void* padding8;
} FcitxAddonAPIFrontend;

#endif // __FCITX_FRONTEND_H__
