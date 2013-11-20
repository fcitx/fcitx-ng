#ifndef FCITX_COMPOSETABLE_TABLEGENERATOR_H
#define FCITX_COMPOSETABLE_TABLEGENERATOR_H

#define FCITX_KEYSEQUENCE_MAX_LEN 7

typedef struct _FcitxComposeTable FcitxComposeTable;

typedef enum _FcitxComposeTableState {
    FCTS_NoErrors = 0, 
    FCTS_UnsupportedLocale,
    FCTS_EmptyTable,
    FCTS_UnknownSystemComposeDir,
    FCTS_MissingComposeFile,
} FcitxComposeTableState;

FcitxComposeTable* fcitx_compose_table_new_from_file(const char* systemComposeDir, const char* composeFile, const char* locale);
FcitxComposeTable* fcitx_compose_table_new(const char* locale);
FcitxComposeTable* fcitx_compose_table_ref(FcitxComposeTable* table);
void fcitx_compose_table_unref(FcitxComposeTable* table);
void fcitx_compose_table_print(FcitxComposeTable* table);
FcitxComposeTableState fcitx_compose_table_state(FcitxComposeTable* table);

#endif // FCITX_COMPOSETABLE_TABLEGENERATOR_H