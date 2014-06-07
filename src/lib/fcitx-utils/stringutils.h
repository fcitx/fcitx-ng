#ifndef _FCITX_UTILS_STRINGUTILS_H_
#define _FCITX_UTILS_STRINGUTILS_H_

#include <fcitx-utils/macro.h>
#include <fcitx-utils/utarray.h>
#include <fcitx-utils/uthash.h>
#include <fcitx-utils/types.h>

FCITX_DECL_BEGIN

char fcitx_utils_unescape_char(char c);
char *fcitx_utils_unescape_str_inplace(char *str);
char *fcitx_utils_set_unescape_str(char *res, const char *str);
#define FCITX_CHAR_NEED_ESCAPE "\a\b\f\n\r\t\e\v\'\"\\"
char fcitx_utils_escape_char(char c);
char *fcitx_utils_set_escape_str_with_set(char *res, const char *str,
                                              const char *set);
static _FCITX_INLINE_ char*
fcitx_utils_set_escape_str(char *res, const char *str)
{
    return fcitx_utils_set_escape_str_with_set(res, str, NULL);
}

char *fcitx_utils_set_str_with_len(char *res, const char *str, size_t len);

static _FCITX_INLINE_ char*
fcitx_utils_set_str(char *res, const char *str)
{
    return fcitx_utils_set_str_with_len(res, str, strlen(str));
}

size_t fcitx_utils_str_lens(size_t n, const char **str_list,
                            size_t *size_list);
void fcitx_utils_cat_str(char *out, size_t n, const char **str_list,
                         const size_t *size_list);
void fcitx_utils_cat_str_with_len(char *out, size_t len, size_t n,
                                  const char **str_list,
                                  const size_t *size_list);
#define fcitx_utils_cat_str_simple(out, n, str_list) do {       \
        size_t __tmp_size_list[n];                              \
        fcitx_utils_str_lens(n, str_list, __tmp_size_list);     \
        fcitx_utils_cat_str(out, n, str_list, __tmp_size_list); \
    } while (0)

#define fcitx_utils_cat_str_simple_with_len(out, len, n, str_list) do { \
        size_t __tmp_size_list[n];                                      \
        fcitx_utils_str_lens(n, str_list, __tmp_size_list);             \
        fcitx_utils_cat_str_with_len(out, len, n, str_list, __tmp_size_list); \
    } while (0)

#define fcitx_utils_local_cat_str(dest, len, strs...)                   \
    const char *__str_list_##dest[] = {strs};                           \
    size_t __size_list_##dest[sizeof((const char*[]){strs}) / sizeof(char*)]; \
    fcitx_utils_str_lens(sizeof((const char*[]){strs}) / sizeof(char*), \
                         __str_list_##dest, __size_list_##dest);        \
    char dest[len];                                                     \
    fcitx_utils_cat_str_with_len(dest, len,                             \
                                 sizeof((const char*[]){strs}) / sizeof(char*), \
                                 __str_list_##dest, __size_list_##dest)

#define fcitx_utils_alloc_cat_str(dest, strs...) do {                   \
        const char *__str_list[] = {strs};                              \
        size_t __cat_str_n = sizeof(__str_list) / sizeof(char*);        \
        size_t __size_list[sizeof(__str_list) / sizeof(char*)];         \
        size_t __total_size = fcitx_utils_str_lens(__cat_str_n,         \
                                                   __str_list, __size_list); \
        dest = malloc(__total_size);                                    \
        fcitx_utils_cat_str(dest, __cat_str_n,                          \
                            __str_list, __size_list);                   \
    } while (0)

#define fcitx_utils_set_cat_str(dest, strs...) do {                     \
        const char *__str_list[] = {strs};                              \
        size_t __cat_str_n = sizeof(__str_list) / sizeof(char*);        \
        size_t __size_list[sizeof(__str_list) / sizeof(char*)];         \
        size_t __total_size = fcitx_utils_str_lens(__cat_str_n,         \
                                                   __str_list, __size_list); \
        dest = realloc(dest, __total_size);                             \
        fcitx_utils_cat_str(dest, __cat_str_n,                          \
                            __str_list, __size_list);                   \
    } while (0)



/**
 * Trim the input string's white space
 *
 * @param s NUL-terminated string
 * @return char* new malloced string, need to free'd by caller
 **/
char* fcitx_utils_trim(const char *s);

/**
 * this function will directly modify the string and return the non space start point
 * and will automatically remove trailling space in-place by null terminator.
 *
 * @param s NUL-terminated string
 * @return start pointer of n
 */
char* fcitx_utils_inplace_trim(char* s);

char* fcitx_utils_backward_search(const char *haystack, size_t l, const char *needle, size_t ol, size_t from);
char* fcitx_utils_strrstr(const char* haystack, const char* needle);

boolean fcitx_utils_string_starts_with(const char* s, const char* needle);
boolean fcitx_utils_string_ends_with(const char* s, const char* needle);

char* fcitx_utils_string_replace(const char* s, const char* before, const char* after, boolean nullIfNoMatch);

FCITX_DECL_END

#endif
