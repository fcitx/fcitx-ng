#include "fcitx/inputcontext.h"

void init_ic(FcitxInputContext* context, void* userData)
{
}

int main()
{
    FcitxInputContextManager* manager = fcitx_input_context_manager_new();
    FcitxInputContext* ic = fcitx_input_context_manager_create_ic(manager, init_ic, NULL);

    fcitx_input_context_manager_focus_in(manager, fcitx_input_context_get_id(ic));

    fcitx_input_context_manager_focus_out(manager, fcitx_input_context_get_id(ic));

    fcitx_input_context_manager_destroy_ic

    return 0;
}
