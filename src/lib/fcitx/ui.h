#ifndef _FCITX_UI_H_
#define _FCITX_UI_H_

#include <fcitx-utils/utils.h>

/** message type and flags */
typedef enum _FcitxTextFormatFlag {
    FTFF_UnderLine = (1 << 0), /**< underline is a flag */
    FTFF_HighLight = (1 << 1), /**< highlight the preedit */
    FTFF_Bold = (1 << 2),
    FTFF_Strike = (1 << 3),
    FTFF_None = 0,
} FcitxTextFormatFlag;

typedef uint32_t FcitxTextFormatFlags;

typedef struct _FcitxText FcitxText;

FcitxText* fcitx_text_new();
FcitxText* fcitx_text_ref(FcitxText* text);
void fcitx_text_unref(FcitxText* text);

void fcitx_text_clear(FcitxText* text);
void fcitx_text_append(FcitxText* text, FcitxTextFormatFlags flag, const char* str);
void fcitx_text_append_sprintf(FcitxText* text, FcitxTextFormatFlags flag, const char* fmt, ...);

size_t fcitx_text_size(FcitxText* text);
const char* fcitx_text_get_string(FcitxText* text, size_t index);
FcitxTextFormatFlags fcitx_text_get_format(FcitxText* text, size_t index);

char* fcitx_text_to_string(FcitxText* text);

#endif // _FCITX_UI_H_
