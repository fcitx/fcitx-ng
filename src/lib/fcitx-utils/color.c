#include "utils.h"
#include "color.h"

#define IsColorValid(c) ((c) >=0 && (c) <= 255)
#define RoundColor(c) ((c)>=0?((c)<=255?c:255):0)

static inline char to_hex_char(int v)
{
    return (char)(((v)>=0 && (v)<10)?(v + '0') : (v - 10 + 'a'));
}

static inline int to_hex_digit(char hi, char lo)
{
    hi = fcitx_utils_tolower(hi);
    lo = fcitx_utils_tolower(lo);
    int dhi = 0, dlo = 0;
    if (hi >= '0' && hi <= '9') {
        dhi = hi - '0';
    } else {
        dhi = hi - 'a' + 10;
    }
    if (lo >= '0' && lo <= '9') {
        dlo = lo - '0';
    } else {
        dlo = lo - 'a' + 10;
    }

    return dhi * 16 + dlo;
}

FCITX_EXPORT_API
bool fcitx_color_parse(FcitxColor* color, const char* str)
{
    size_t idx = 0;

    // skip space
    while (str[idx] && fcitx_utils_isspace(str[idx])) {
        idx ++;
    }

    if (str[idx] == '#') {
        // count the digit length
        size_t len = 0;
        const char* digits = &str[idx + 1];
        while(digits[len] &&
            (fcitx_utils_isdigit(digits[len]) ||
             ('A' <= digits[len] && digits[len] <= 'F') |
             ('a' <= digits[len] && digits[len] <= 'f'))) {
            len++;
        }
        if (len != 8) {
            return false;
        }

        int r, g, b, a;
        r = to_hex_digit(digits[0], digits[1]);
        digits += 2;
        g = to_hex_digit(digits[0], digits[1]);
        digits += 2;
        b = to_hex_digit(digits[0], digits[1]);
        digits += 2;
        a = to_hex_digit(digits[0], digits[1]);

        if (IsColorValid(r) && IsColorValid(g) && IsColorValid(b) && IsColorValid(a)) {
            color->red = r / 255.0;
            color->green = g / 255.0;
            color->blue = b / 255.0;
            color->alpha = a / 255.0;
            return true;
        }
    } else {
        int r, g, b;
        if (sscanf(str, "%d %d %d", &r, &g, &b) != 3) {
            return false;
        }

        if (IsColorValid(r) && IsColorValid(g) && IsColorValid(b)) {
            color->red = r / 255.0;
            color->green = g / 255.0;
            color->blue = b / 255.0;
            color->alpha = 1.0;
            return true;
        }
    }

    return false;
}

FCITX_EXPORT_API
void fcitx_color_to_string(const FcitxColor* color, char* str)
{
    str[0] = '#';
    str++;
    int v[4];
    v[0] = (int)(color->red * 255);
    v[1] = (int)(color->green * 255);
    v[2] = (int)(color->blue * 255);
    v[3] = (int)(color->alpha * 255);
    for (size_t i = 0; i < 4; i ++) {
        int value = RoundColor(v[i]);
        int hi = value / 16;
        int lo = value % 16;
        *str = to_hex_char(hi);
        str++;
        *str = to_hex_char(lo);
        str++;
    }

    *str = '\0';
}
