#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "fcitx-composetable/tablegenerator.h"
#include "fcitx-composetable/composecontext.h"

int main()
{
    FcitxComposeTable* table = fcitx_compose_table_new(NULL);
    // fcitx_compose_table_print(table);
    FcitxComposeContext* context = fcitx_compose_context_new(table);
    assert(fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_Multi_key, 0)));
    assert(fcitx_compose_context_get_char(context) == 0);
    assert(fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_o, 0)));
    assert(fcitx_compose_context_get_char(context) == 0);
    assert(fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_c, 0)));
    assert(fcitx_compose_context_get_char(context) == 0xa9);
    assert(fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_Multi_key, 0)));
    assert(fcitx_compose_context_get_char(context) == 0);
    assert(fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_equal, 0)));
    assert(fcitx_compose_context_get_char(context) == 0);
    assert(fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_c, 0)));
    assert(fcitx_compose_context_get_char(context) == 0);
    assert(!fcitx_compose_context_process_key(context, FCITX_KEY(FcitxKey_c, 0)));
    fcitx_compose_table_unref(table);
    fcitx_compose_context_unref(context);
    return 0;
}