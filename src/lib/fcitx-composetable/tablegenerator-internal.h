#ifndef FCITX_COMPOSE_TABLE_TABLEGENERATOR_INTERNAL_H
#define FCITX_COMPOSE_TABLE_TABLEGENERATOR_INTERNAL_H

#include "tablegenerator.h"
#include "fcitx-utils/dict.h"
#include "fcitx-utils/utarray.h"
#include "fcitx-utils/keysymgen.h"

typedef struct _FcitxComposeTableElement {
    FcitxKeySym keys[FCITX_KEYSEQUENCE_MAX_LEN];
    uint32_t value;
} FcitxComposeTableElement;

struct _FcitxComposeTable
{
    int32_t refcount;
    char* systemComposeDir;
    FcitxDict* localeToTable;
    FcitxComposeTableState state;
    UT_array* composeTable;
    char* locale;
};

int fcitx_compose_table_element_cmp(const void* a, const void* b, void* data);

#endif // FCITX_COMPOSE_TABLE_TABLEGENERATOR_INTERNAL_H