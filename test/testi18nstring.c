#include <assert.h>
#include "fcitx-utils/utils.h"

int main()
{
    FcitxI18NString* s = fcitx_i18n_string_new();
    fcitx_dict_insert_by_str(s, "zh", strdup("TEST1"), false);
    fcitx_dict_insert_by_str(s, "zh_CN", strdup("TEST2"), false);

    assert(strcmp("TEST2", fcitx_i18n_string_match(s, "zh_CN@whatever")) == 0);
    assert(strcmp("TEST2", fcitx_i18n_string_match(s, "zh_CN")) == 0);
    assert(strcmp("TEST1", fcitx_i18n_string_match(s, "zh_TW")) == 0);
    assert(strcmp("TEST2", fcitx_i18n_string_match(s, "zh_CN.UTF-8")) == 0);
    assert(strcmp("TEST2", fcitx_i18n_string_match(s, "zh_CN.utf8")) == 0);

    fcitx_i18n_string_free(s);

    return 0;
}
