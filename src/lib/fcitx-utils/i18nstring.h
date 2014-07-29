#ifndef _FCITX_UTILS_I18NSTRING_H_
#define _FCITX_UTILS_I18NSTRING_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include "dict.h"

typedef FcitxDict FcitxI18NString;

FcitxI18NString* fcitx_i18n_string_new();

const char* fcitx_i18n_string_match(FcitxI18NString* s, const char* locale);

static inline void fcitx_i18n_string_free(FcitxI18NString* s) {
    fcitx_dict_free(s);
}

#endif // _FCITX_UTILS_I18NSTRING_H_
