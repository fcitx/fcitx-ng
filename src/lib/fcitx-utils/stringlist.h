#ifndef _FCITX_UTILS_STRING_LIST_H_
#define _FCITX_UTILS_STRING_LIST_H_

#include <fcitx-utils/types.h>
#include <fcitx-utils/macro.h>
#include <fcitx-utils/utarray.h>

typedef UT_array FcitxStringList;

/**
 * create empty string list
 *
 * @return FcitxStringList*
 **/
FcitxStringList* fcitx_utils_string_list_new(void);

/**
 * Split a string by delm
 *
 * @param str input string
 * @param delm character as delimiter
 * @return FcitxStringList* a new utarray for store the split string
 **/
FcitxStringList* fcitx_utils_string_split(const char *str, const char* delm);

/**
 * append a string with printf format
 *
 * @param list string list
 * @param fmt printf fmt
 * @return void
 **/
void fcitx_utils_string_list_printf_append(FcitxStringList* list, const char* fmt,...);

/**
 * Join string list with delm
 *
 * @param list string list
 * @param delm delm
 * @return char* return string, need to be free'd
 **/
char* fcitx_utils_string_list_join(FcitxStringList* list, char delm);

/**
 * check if a string list contains a specific string
 *
 * @param list string list
 * @param scmp string to compare
 *
 * @return 1 for found, 0 for not found.
 *
 * @since 4.2.5
 */
int fcitx_utils_string_list_contains(FcitxStringList* list, const char* scmp);

/**
 * Helper function for free the SplitString Output
 *
 * @param list the SplitString Output
 * @return void
 * @see fcitx_utils_split_string
 **/
void fcitx_utils_string_list_free(FcitxStringList *list);

FcitxStringList *fcitx_utils_string_list_append_no_copy(FcitxStringList *list, char *str);
FcitxStringList *fcitx_utils_string_list_append_len(FcitxStringList *list,
                                                    const char *str, size_t len);
FcitxStringList *fcitx_utils_string_list_append_split(FcitxStringList *list, const char* str,
                                                      const char *delm);

static inline FcitxStringList*
fcitx_utils_string_list_append_lines(FcitxStringList *list, const char* str)
{
    return fcitx_utils_string_list_append_split(list, str, "\n");
}

FCITX_DECL_END

#endif // _FCITX_UTILS_STRING_LIST_H_
