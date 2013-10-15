#include "stringutils.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

FCITX_EXPORT_API char*
fcitx_utils_set_str_with_len(char *res, const char *str, size_t len)
{
    if (res) {
        res = realloc(res, len + 1);
    } else {
        res = malloc(len + 1);
    }
    memcpy(res, str, len);
    res[len] = '\0';
    return res;
}


FCITX_EXPORT_API char*
fcitx_utils_set_escape_str_with_set(char *res, const char *str, const char *set)
{
    if (!set)
        set = FCITX_CHAR_NEED_ESCAPE;
    size_t len = strlen(str) * 2 + 1;
    if (res) {
        res = realloc(res, len);
    } else {
        res = malloc(len);
    }
    char *dest = res;
    const char *src = str;
    const char *pos;
    while ((len = strcspn(src, set)), *(pos = src + len)) {
        memcpy(dest, src, len);
        dest += len;
        *dest = '\\';
        dest++;
        *dest = fcitx_utils_escape_char(*pos);
        dest++;
        src = pos + 1;
    }
    if (len)
        memcpy(dest, src, len);
    dest += len;
    *dest = '\0';
    res = realloc(res, dest - res + 1);
    return res;
}

FCITX_EXPORT_API size_t
fcitx_utils_str_lens(size_t n, const char **str_list, size_t *size_list)
{
    size_t i;
    size_t total = 0;
    for (i = 0;i < n;i++) {
        total += (size_list[i] = str_list[i] ? strlen(str_list[i]) : 0);
    }
    return total + 1;
}

FCITX_EXPORT_API void
fcitx_utils_cat_str(char *out, size_t n, const char **str_list,
                        const size_t *size_list)
{
    size_t i = 0;
    for (i = 0;i < n;i++) {
        if (!size_list[i])
            continue;
        memcpy(out, str_list[i], size_list[i]);
        out += size_list[i];
    }
    *out = '\0';
}

FCITX_EXPORT_API void
fcitx_utils_cat_str_with_len(char *out, size_t len, size_t n,
                             const char **str_list, const size_t *size_list)
{
    char *limit = out + len - 1;
    char *tmp = out;
    size_t i = 0;
    for (i = 0;i < n;i++) {
        if (!size_list[i])
            continue;
        tmp += size_list[i];
        if (tmp > limit) {
            memcpy(out, str_list[i], limit - out);
            out = limit;
            break;
        }
        memcpy(out, str_list[i], size_list[i]);
        out = tmp;
    }
    *out = '\0';
}

FCITX_EXPORT_API char
fcitx_utils_unescape_char(char c)
{
    switch (c) {
#define CASE_UNESCAPE(from, to) case from: return to
        CASE_UNESCAPE('a', '\a');
        CASE_UNESCAPE('b', '\b');
        CASE_UNESCAPE('f', '\f');
        CASE_UNESCAPE('n', '\n');
        CASE_UNESCAPE('r', '\r');
        CASE_UNESCAPE('t', '\t');
        CASE_UNESCAPE('e', '\e');
        CASE_UNESCAPE('v', '\v');
#undef CASE_UNESCAPE
    }
    return c;
}

FCITX_EXPORT_API char
fcitx_utils_escape_char(char c)
{
    switch (c) {
#define CASE_ESCAPE(to, from) case from: return to
        CASE_ESCAPE('a', '\a');
        CASE_ESCAPE('b', '\b');
        CASE_ESCAPE('f', '\f');
        CASE_ESCAPE('n', '\n');
        CASE_ESCAPE('r', '\r');
        CASE_ESCAPE('t', '\t');
        CASE_ESCAPE('e', '\e');
        CASE_ESCAPE('v', '\v');
#undef CASE_ESCAPE
    }
    return c;
}

FCITX_EXPORT_API char*
fcitx_utils_set_unescape_str(char *res, const char *str)
{
    size_t len = strlen(str) + 1;
    if (res) {
        res = realloc(res, len);
    } else {
        res = malloc(len);
    }
    char *dest = res;
    const char *src = str;
    const char *pos;
    while ((len = strcspn(src, "\\")), *(pos = src + len)) {
        memcpy(dest, src, len);
        dest += len;
        src = pos + 1;
        *dest = fcitx_utils_unescape_char(*src);
        dest++;
        src++;
    }
    if (len)
        memcpy(dest, src, len);
    dest[len] = '\0';
    return res;
}

FCITX_EXPORT_API char*
fcitx_utils_unescape_str_inplace(char *str)
{
    char *dest = str;
    char *src = str;
    char *pos;
    size_t len;
    while ((len = strcspn(src, "\\")), *(pos = src + len)) {
        if (dest != src && len)
            memmove(dest, src, len);
        dest += len;
        src = pos + 1;
        *dest = fcitx_utils_unescape_char(*src);
        dest++;
        src++;
    }
    if (dest != src && len)
        memmove(dest, src, len);
    dest[len] = '\0';
    return str;
}


FCITX_EXPORT_API
char* fcitx_utils_trim(const char* s)
{
    register const char *end;

    s += strspn(s, "\f\n\r\t\v ");
    end = s + (strlen(s) - 1);
    while (end >= s && isspace(*end))               /* skip trailing space */
        --end;

    end++;

    size_t len = end - s;

    char* result = malloc(len + 1);
    memcpy(result, s, len);
    result[len] = '\0';
    return result;
}

#define REHASH(a) \
    if (ol_minus_1 < sizeof(uint) * CHAR_BIT) \
        hashHaystack -= (a) << ol_minus_1; \
    hashHaystack <<= 1

FCITX_EXPORT_API
char* fcitx_utils_backward_search(const char* haystack, size_t l, const char* needle, size_t ol, size_t from)
{
    if (ol > l) {
        return NULL;
    }
    size_t delta = l - ol;
    if (from > l)
        return NULL;
    if (from > delta)
        from = delta;

    const char *end = haystack;
    haystack += from;
    const uint ol_minus_1 = ol - 1;
    const char *n = needle + ol_minus_1;
    const char *h = haystack + ol_minus_1;
    uint hashNeedle = 0, hashHaystack = 0;
    size_t idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + *(n-idx));
        hashHaystack = ((hashHaystack<<1) + *(h-idx));
    }
    hashHaystack -= *haystack;
    while (haystack >= end) {
        hashHaystack += *haystack;
        if (hashHaystack == hashNeedle && memcmp(needle, haystack, ol) == 0)
            return (char*) haystack;
        --haystack;
        REHASH(*(haystack + ol));
    }
    return NULL;
}

FCITX_EXPORT_API
char* fcitx_utils_strrstr(const char* haystack, const char* needle)
{
    if (needle[0] == '\0') {
        return NULL;
    }

    if (needle[1] == '\0') {
        return strrchr(haystack, needle[0]);
    }

    int l = strlen(haystack);
    int ol = strlen(needle);
    return fcitx_utils_backward_search(haystack, l, needle, ol, 0);
}
