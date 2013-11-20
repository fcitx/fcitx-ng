#include "composecontext.h"
#include "tablegenerator-internal.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/atomic.h"
#include <fcitx-utils/utf8.h>
#include <stdlib.h>

struct _FcitxComposeContext
{
    FcitxComposeTable* table;
    FcitxKeySym composeBuffer[FCITX_KEYSEQUENCE_MAX_LEN];
    int32_t refcount;
    uint32_t unicode;
    char utf8Buffer[FCITX_UTF8_MAX_LENGTH];
};

boolean is_composing_key (FcitxKeySym sym) {
    switch (sym) {
        case FcitxKey_Multi_key:
        case FcitxKey_dead_grave:
        case FcitxKey_dead_acute:
        case FcitxKey_dead_circumflex:
        case FcitxKey_dead_tilde:
        case FcitxKey_dead_macron:
        case FcitxKey_dead_breve:
        case FcitxKey_dead_abovedot:
        case FcitxKey_dead_diaeresis:
        case FcitxKey_dead_abovering:
        case FcitxKey_dead_doubleacute:
        case FcitxKey_dead_caron:
        case FcitxKey_dead_cedilla:
        case FcitxKey_dead_ogonek:
        case FcitxKey_dead_iota:
        case FcitxKey_dead_voiced_sound:
        case FcitxKey_dead_semivoiced_sound:
        case FcitxKey_dead_belowdot:
        case FcitxKey_dead_hook:
        case FcitxKey_dead_horn:
            return true;
        default:
            return false;
    }
}

FCITX_EXPORT_API
FcitxComposeContext* fcitx_compose_context_new(FcitxComposeTable* table)
{
    FcitxComposeContext* context = fcitx_utils_new(FcitxComposeContext);
    context->table = fcitx_compose_table_ref(table);
    fcitx_utils_atomic_add(&context->refcount, 1);
    return context;
}

void fcitx_compose_context_free(FcitxComposeContext* context)
{
    fcitx_compose_table_unref(context->table);
    free(context);
}

FCITX_EXPORT_API
FcitxComposeContext* fcitx_compose_context_ref(FcitxComposeContext* context)
{
    fcitx_utils_atomic_add (&context->refcount, 1);
    return context;
}

FCITX_EXPORT_API
void fcitx_compose_context_unref(FcitxComposeContext* context)
{
    int32_t oldvalue = fcitx_utils_atomic_add (&context->refcount, -1);
    if (oldvalue == 1) {
        fcitx_compose_context_free(context);
    }
}


int compose_cmp(const void* a, const void* b)
{
    const FcitxComposeContext* c = a;
    const FcitxComposeTableElement* e = b;
    for (size_t i = 0; i < FCITX_KEYSEQUENCE_MAX_LEN; i++) {
        if (c->composeBuffer[i] != e->keys[i]) {
            return (c->composeBuffer[i] < e->keys[i]) ? -1 : 1;
        }
    }
    return 0;
}


boolean _fcitx_compose_context_check_table(FcitxComposeContext* context)
{
    FcitxComposeTableElement* elem = utarray_custom_bsearch(context, context->table->composeTable, false, compose_cmp);

    // prevent dereferencing an 'end' iterator, which would result in a crash
    if (!elem) {
        elem = utarray_back(context->table->composeTable);
    }

    // would be nicer if qLowerBound had API that tells if the item was actually found
    if (context->composeBuffer[0] != elem->keys[0]) {
        fcitx_compose_context_reset(context);
        return false;
    }
    // check if compose buffer is matched
    for (int i = 0; i < FCITX_KEYSEQUENCE_MAX_LEN; i++) {

        // check if partial match
        if (context->composeBuffer[i] == 0 && elem->keys[i]) {
            return true;
        }

        if (context->composeBuffer[i] != elem->keys[i]) {
            fcitx_compose_context_reset(context);
            return i != 0;
        }
    }

    // check if the key sequence is overwriten
    do {
        FcitxComposeTableElement* nextElem;
        while ((nextElem = utarray_next(context->table->composeTable, elem)) != NULL) {
            if (fcitx_compose_table_element_cmp(elem, nextElem, NULL) == 0) {
                elem = nextElem;
            } else {
                break;
            }
        }
        break;
    } while (true);

    context->unicode = elem->value;
    fcitx_compose_context_reset(context);

    return true;
}

FCITX_EXPORT_API
boolean fcitx_compose_context_process_key(FcitxComposeContext* context, FcitxKey key)
{
    context->unicode = 0;
    if (fcitx_compose_table_state(context->table) != FCTS_NoErrors) {
        return false;
    }

    if (FcitxKeyIsModifierCombine(key))
        return false;
    
    if (key.state & FcitxKeyState_SimpleMask) {
        return false;
    }

    if (!is_composing_key(key.sym) && FcitxKeySymToUnicode(key.sym) == 0)
        return false;

    int nCompose = 0;
    while (context->composeBuffer[nCompose] != 0 && nCompose < FCITX_KEYSEQUENCE_MAX_LEN)
        nCompose++;

    if (nCompose == FCITX_KEYSEQUENCE_MAX_LEN) {
        fcitx_compose_context_reset(context);
        nCompose = 0;
    }

    context->composeBuffer[nCompose] = key.sym;
    // check sequence
    if (_fcitx_compose_context_check_table(context))
        return true;

    return false;
}

FCITX_EXPORT_API
uint32_t fcitx_compose_context_get_char(FcitxComposeContext* context)
{
    return context->unicode;
}

FCITX_EXPORT_API
const char* fcitx_compose_context_get_text(FcitxComposeContext* context)
{
    if (context->unicode) {
        fcitx_ucs4_to_utf8(context->unicode, context->utf8Buffer);
    }
    return context->utf8Buffer;
}

FCITX_EXPORT_API
void fcitx_compose_context_reset(FcitxComposeContext* context)
{
    for (size_t i = 0; i < FCITX_ARRAY_SIZE(context->composeBuffer); i ++) {
        context->composeBuffer[i] = 0;
    }
}


