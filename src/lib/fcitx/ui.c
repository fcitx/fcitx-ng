#include "ui.h"
#include "fcitx-utils/utils.h"
#include "fcitx-utils/macro-internal.h"

typedef struct _FcitxTextString
{
    char* string;
    FcitxTextFormatFlags format;
} FcitxTextString;

struct _FcitxText
{
    int refcount;
    UT_array strings;
};

void fcitx_text_string_free(void* data)
{
    FcitxTextString* s = data;
    free(s->string);
}

static const UT_icd text_string = {sizeof(FcitxTextString), NULL, NULL, fcitx_text_string_free};

FCITX_EXPORT_API
FcitxText* fcitx_text_new()
{
    FcitxText* text = fcitx_utils_new(FcitxText);
    utarray_init(&text->strings, &text_string);
    return fcitx_text_ref(text);
}

void fcitx_text_free(FcitxText* text)
{
    free(text);
}

FCITX_REFCOUNT_FUNCTION_DEFINE(FcitxText, fcitx_text)

FCITX_EXPORT_API
void fcitx_text_append(FcitxText* text, FcitxTextFormatFlags flag, const char* str)
{
    FcitxTextString s;
    s.format = flag;
    s.string = strdup(str);

    utarray_push_back(&text->strings, &s);
}

FCITX_EXPORT_API
void fcitx_text_append_sprintf(FcitxText* text, FcitxTextFormatFlags flag, const char* fmt, ...)
{
    FcitxTextString s;
    s.format = flag;
    va_list va;
    va_start(va, fmt);
    fcitx_vasprintf(&s.string, fmt, va);
    va_end(va);

    utarray_push_back(&text->strings, &s);
}

FCITX_EXPORT_API
void fcitx_text_clear(FcitxText* text)
{
    utarray_clear(&text->strings);
}

size_t fcitx_text_size(FcitxText* text)
{
    return utarray_len(&text->strings);
}

FcitxTextFormatFlags fcitx_text_get_format(FcitxText* text, size_t index)
{
    return ((FcitxTextString*) utarray_eltptr(&text->strings, index))->format;
}

const char* fcitx_text_get_string(FcitxText* text, size_t index)
{
    return ((FcitxTextString*) utarray_eltptr(&text->strings, index))->string;
}

char* fcitx_text_to_string(FcitxText* text)
{
    size_t length = 0;
    for (size_t i = 0; i < utarray_len(&text->strings); i++) {
        const char *str = ((FcitxTextString*) utarray_eltptr(&text->strings, index))->string;
        length += strlen(str);
    }

    char *str = fcitx_utils_malloc0(sizeof(char) * (length + 1));
    char* p = str;

    for (size_t i = 0; i < utarray_len(&text->strings); i++) {
        const char *str = ((FcitxTextString*) utarray_eltptr(&text->strings, index))->string;
        p = stpcpy(p, str);
    }

    return str;
}
