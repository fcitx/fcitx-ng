#ifndef FCITX_IM_TABLEDICT_H
#define FCITX_IM_TABLEDICT_H

#include <fcitx-utils/macro.h>
#include <fcitx-utils/types.h>
#include <stdio.h>

FCITX_DECL_BEGIN

typedef struct _FcitxTableDict FcitxTableDict;

typedef enum _FcitxTableDictParseError
{
    FTDPE_NoError = 0,
    FTDPE_ValidCharsInvalid,
    FTDPE_ValidCharsMissing,
    FTDPE_CodeLengthInvalid,
    FTDPE_CodeLengthMissing,
    FTDPT_RuleInvalid,
    FTDPE_NoDataSection,
    FTDPE_Corrupted
} FcitxTableDictParseError;

FcitxTableDict* fcitx_table_dict_new();

boolean fcitx_table_dict_load_text(FcitxTableDict* tableDict, FILE* fp);
boolean fcitx_table_dict_save_text(FcitxTableDict* tableDict, FILE* fp);

boolean fcitx_table_dict_load_binary(FcitxTableDict* tableDict, FILE* fp);
boolean fcitx_table_dict_save_binary(FcitxTableDict* tableDict, FILE* fp);

boolean fcitx_table_dict_load_symbol(FcitxTableDict* tableDict, const char* file);
boolean fcitx_table_dict_load_auto_phrase_file(FcitxTableDict* tableDict, const char* filename);

boolean fcitx_table_dict_construct_word(FcitxTableDict* tableDict, const char *word, char* newWordCode);

uint8_t fcitx_table_dict_get_code_length(FcitxTableDict* tableDict);
const char* fcitx_table_dict_get_valid_chars(FcitxTableDict* tableDict);
const char* fcitx_table_dict_get_ignored_chars(FcitxTableDict* tableDict);

FcitxTableDict* fcitx_table_dict_ref(FcitxTableDict* tableDict);
void fcitx_table_dict_unref(FcitxTableDict* tableDict);

FCITX_DECL_END

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0;
