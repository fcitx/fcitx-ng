#include "fcitx-utils/utils.h"
#include "inputcontext-internal.h"

FcitxInputContext* FcitxInputContextNew(FcitxFrontend* frontend)
{
    return fcitx_utils_new(FcitxInputContext);
}

void FcitxInputContextProcessKeyEvent(FcitxInputContext* inputContext, FcitxKeyEvent* event)
{

}
