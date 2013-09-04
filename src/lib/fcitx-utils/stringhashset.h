#ifndef _FCITX_UTILS_STRING_HASHSET_H_
#define _FCITX_UTILS_STRING_HASHSET_H_

#include <fcitx-utils/macro.h>
#include <fcitx-utils/uthash.h>
#include <fcitx-utils/types.h>

FCITX_DECL_BEGIN

typedef struct _FcitxStringHashSet {
    /**
     * String in Hash Set
     **/
    char *str;
    /**
     * UT Hash handle
     **/
    UT_hash_handle hh;
} FcitxStringHashSet;


/**
 * Free String Hash Set
 *
 * @param sset String Hash Set
 * @return void
 *
 * @since 4.2.0
 **/
void fcitx_utils_string_hash_set_free(FcitxStringHashSet* sset);


/**
 * compare two string with strcmp
 *
 * @param sseta left
 * @param ssetb right
 * @return same as strcmp
 *
 * @since 4.2.8
 **/
int fcitx_utils_string_hash_set_compare(FcitxStringHashSet* sseta, FcitxStringHashSet* ssetb);

/**
 * insert to a string hash set
 *
 * @param sset string hash set
 * @param str string
 * @return FcitxStringHashSet*
 *
 * @since 4.2.7
 **/
FcitxStringHashSet* fcitx_utils_string_hash_set_insert(FcitxStringHashSet* sset, const char* str);


/**
 * insert string with specified length
 *
 * @param sset string hash set
 * @param str string
 * @param len length
 * @return FcitxStringHashSet*
 *
 * @since 4.2.7
 **/
FcitxStringHashSet* fcitx_utils_string_hash_set_insert_len(FcitxStringHashSet* sset, const char* str, size_t len);

/**
 * check a string contains in string hash set or not
 *
 * @param sset string hash set
 * @param str string
 * @return boolean
 *
 * @since 4.2.7
 **/
boolean fcitx_utils_string_hash_set_contains(FcitxStringHashSet* sset, const char* str);

/**
 * remove a string from string hash set
 *
 * @param sset string hash set
 * @param str string
 * @return FcitxStringHashSet*
 *
 * @since 4.2.7
 **/
FcitxStringHashSet* fcitx_utils_string_hash_set_remove(FcitxStringHashSet* sset, const char* str);

/**
 * join a string hash set with delimiter
 *
 * @param sset string hash set
 * @param delim delimeter
 * @return char*
 *
 * @since 4.2.7
 **/
char* fcitx_utils_string_hash_set_join(FcitxStringHashSet* sset, char delim);

/**
 * parse a string with delimiter
 *
 * @param str string
 * @param delim delimiter
 * @return FcitxStringHashSet*
 *
 * @since 4.2.7
 **/
FcitxStringHashSet* fcitx_utils_string_hash_set_parse(const char* str, char delim);

FCITX_DECL_END

#endif
