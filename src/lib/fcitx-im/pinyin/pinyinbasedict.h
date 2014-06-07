#ifndef FCITX_IM_PINYIN_BASE_DICT_H
#define FCITX_IM_PINYIN_BASE_DICT_H

#include <fcitx-utils/types.h>
#include <stdio.h>

typedef struct _FcitxPinyinBaseDict FcitxPinyinBaseDict;

FcitxPinyinBaseDict* fcitx_pinyin_base_dict_new();

boolean fcitx_pinyin_base_dict_load_text(FcitxPinyinBaseDict* tableDict, FILE* fp);
boolean fcitx_pinyin_base_dict_save_text(FcitxPinyinBaseDict* tableDict, FILE* fp);

boolean fcitx_pinyin_base_dict_load_binary(FcitxPinyinBaseDict* tableDict, FILE* fp);
boolean fcitx_pinyin_base_dict_save_binary(FcitxPinyinBaseDict* tableDict, FILE* fp);

void fcitx_pinyin_base_dict_unref(FcitxPinyinBaseDict* tableDict);

#endif
