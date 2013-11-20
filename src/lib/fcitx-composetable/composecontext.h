#ifndef FCITX_COMPSOTETABLE_COMPOSECONTEXT_H
#define FCITX_COMPSOTETABLE_COMPOSECONTEXT_H
#include "tablegenerator.h"
#include <fcitx-utils/types.h>
#include <fcitx-utils/key.h>

typedef struct _FcitxComposeContext FcitxComposeContext;

FcitxComposeContext* fcitx_compose_context_new(FcitxComposeTable* table);
FcitxComposeContext* fcitx_compose_context_ref(FcitxComposeContext* context);
void fcitx_compose_context_unref(FcitxComposeContext* context);
boolean fcitx_compose_context_process_key(FcitxComposeContext* context, FcitxKey key);
uint32_t fcitx_compose_context_get_char(FcitxComposeContext* context);
const char* fcitx_compose_context_get_text(FcitxComposeContext* context);
void fcitx_compose_context_reset(FcitxComposeContext* context);

#endif // FCITX_COMPSOTETABLE_COMPOSECONTEXT_H