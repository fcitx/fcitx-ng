#ifndef FCITX_IM_PINYINDICT_H
#define FCITX_IM_PINYINDICT_H


typedef struct _FcitxPinyinDict FcitxPinyinDict;

typedef enum _FcitxPinyinDictParseError
{
    FPDPE_NoError = 0,
    FPDPE_ValidCharsInvalid,
    FPDPE_ValidCharsMissing,
    FPDPE_CodeLengthInvalid,
    FPDPE_CodeLengthMissing,
    FPDPT_RuleInvalid,
    FPDPE_NoDataSection,
    FPDPE_Corrupted
} FcitxPinyinDictParseError;

FcitxPinyinDict* fcitx_pinyindict_new();

boolean fcitx_pinyindict_load_text(FcitxPinyinDict* pinyinDict, FILE* fp);
boolean fcitx_pinyindict_save_text(FcitxPinyinDict* pinyinDict, FILE* fp);

boolean fcitx_pinyindict_load_binary(FcitxPinyinDict* pinyinDict, FILE* fp);
boolean fcitx_pinyindict_save_binary(FcitxPinyinDict* pinyinDict, FILE* fp);

boolean fcitx_pinyindict_load_symbol(FcitxPinyinDict* pinyinDict, const char* file);
boolean fcitx_pinyindict_load_auto_phrase_file(FcitxPinyinDict* pinyinDict, const char* filename);

boolean fcitx_pinyindict_construct_word(FcitxPinyinDict* pinyinDict, const char *word, char* newWordCode);

uint8_t fcitx_pinyindict_get_code_length(FcitxPinyinDict* pinyinDict);
const char* fcitx_pinyindict_get_valid_chars(FcitxPinyinDict* pinyinDict);
const char* fcitx_pinyindict_get_ignored_chars(FcitxPinyinDict* pinyinDict);

FcitxPinyinDict* fcitx_pinyindict_ref(FcitxPinyinDict* pinyinDict);
void fcitx_pinyindict_unref(FcitxPinyinDict* pinyinDict);

#endif
