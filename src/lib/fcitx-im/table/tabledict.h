#ifndef FCITX_IM_TABLEDICT_H
#define FCITX_IM_TABLEDICT_H

#include <fcitx-utils/macro.h>
#include <fcitx-utils/types.h>
#include <stdio.h>

FCITX_DECL_BEGIN

typedef struct _FcitxTableDict FcitxTableDict;

FcitxTableDict* fcitx_tabledict_new();

boolean fcitx_tabledict_load(FcitxTableDict* tableDict, FILE* fp);
boolean fcitx_tabledict_load_symbol(FcitxTableDict* tableDict, const char* file);
boolean fcitx_tabledict_load_auto_phrase_file(FcitxTableDict* tableDict, const char* filename);

void fcitx_tabledict_save(FcitxTableDict* tableDict, const char* file);

const char* fcitx_tabledict_get_valid_chars(FcitxTableDict* tableDict);
const char* fcitx_tabledict_get_ignored_chars(FcitxTableDict* tableDict);

FcitxTableDict* fcitx_tabledict_ref(FcitxTableDict* tableDict);
void fcitx_tabledict_unref(FcitxTableDict* tableDict);

FCITX_DECL_END

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0;
