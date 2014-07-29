#include <assert.h>
#include "fcitx-utils/utils.h"

#define TEST_STR "a,b,c,d"

void test_string()
{
    const char *test = TEST_STR;

    FcitxStringList* list = fcitx_utils_string_split(test, ",");
    assert(utarray_len(list) == 4);
    assert(fcitx_utils_string_list_contains(list, "a"));
    assert(fcitx_utils_string_list_contains(list, "b"));
    assert(fcitx_utils_string_list_contains(list, "c"));
    assert(fcitx_utils_string_list_contains(list, "d"));
    assert(!fcitx_utils_string_list_contains(list, "e"));
    char* join = fcitx_utils_string_list_join(list, ',');
    assert(strcmp(join, test) == 0);
    fcitx_utils_string_list_append_split(list, TEST_STR, "\n");
    fcitx_utils_string_list_printf_append(list, "%s", TEST_STR);
    char *join2 = fcitx_utils_string_list_join(list, ',');
    assert(strcmp(join2, TEST_STR","TEST_STR","TEST_STR) == 0);

    char* cat = NULL;
    fcitx_utils_alloc_cat_str(cat, join, ",e");
    assert(strcmp(cat, TEST_STR",e") == 0);
    fcitx_utils_set_cat_str(cat, join, ",e,", join);
    assert(strcmp(cat, TEST_STR",e,"TEST_STR) == 0);
    join = fcitx_utils_set_str(join, join2);
    assert(strcmp(join, join2) == 0);

    free(cat);
    free(join);
    free(join2);
    fcitx_utils_string_list_free(list);
    
    list = fcitx_utils_string_split_full("a   b", " ", false);
    assert(utarray_len(list) == 2);
    fcitx_utils_string_list_free(list);

    char localcat[20];
    const char *array[] = {"a", ",b", ",c", ",d"};
    fcitx_utils_cat_str_simple(localcat, 4, array);
    assert(strcmp(localcat, test) == 0);

    char localcat2[6];
    fcitx_utils_cat_str_simple_with_len(localcat2, 4, 4, array);
    assert(strcmp(localcat2, "a,b") == 0);

    fcitx_utils_cat_str_simple_with_len(localcat2, 5, 4, array);
    assert(strcmp(localcat2, "a,b,") == 0);

    fcitx_utils_cat_str_simple_with_len(localcat2, 6, 4, array);
    assert(strcmp(localcat2, "a,b,c") == 0);

    const char *orig = "\r78\"1\n\\\e\\3\f\a\v\'cc\td\b";
    char *escape = fcitx_utils_set_escape_str(NULL, orig);
    assert(strcmp(escape,
                  "\\r78\\\"1\\n\\\\\\e\\\\3\\f\\a\\v\\\'cc\\td\\b") == 0);
    char *back = fcitx_utils_set_unescape_str(NULL, escape);
    assert(strcmp(orig, back) == 0);
    fcitx_utils_unescape_str_inplace(escape);
    assert(strcmp(orig, escape) == 0);
    free(escape);
    free(back);
    
    
    char* replace_result = fcitx_utils_string_replace("abcabc", "a", "b", true);
    assert(strcmp(replace_result, "bbcbbc") == 0);
    free(replace_result);
    
#define REPEAT 2049
    char largeReplace[3 * REPEAT + 1];
    char largeReplaceCorrect[REPEAT + 1];
    char largeReplaceCorrect2[4 * REPEAT + 1];
    int i = 0, j = 0, k = 0;
    for (int n = 0; n < REPEAT; n ++) {
        largeReplace[i++] = 'a';
        largeReplace[i++] = 'b';
        largeReplace[i++] = 'c';
        
        largeReplaceCorrect[j++] = 'e';
        
        largeReplaceCorrect2[k++] = 'a';
        largeReplaceCorrect2[k++] = 'b';
        largeReplaceCorrect2[k++] = 'c';
        largeReplaceCorrect2[k++] = 'd';
    }
    
    largeReplace[i] = '\0';
    largeReplaceCorrect[j] = '\0';
    largeReplaceCorrect2[k] = '\0';
    
    replace_result = fcitx_utils_string_replace(largeReplace, "abc", "e", true);
    assert(strcmp(replace_result, largeReplaceCorrect) == 0);
    char* replace_result2 = fcitx_utils_string_replace(replace_result, "e", "abcd", true);
    free(replace_result);
    assert(strcmp(replace_result2, largeReplaceCorrect2) == 0);
    free(replace_result2);
    
    assert(fcitx_utils_string_replace(largeReplace, "de", "bcd", true) == NULL);
    
}

void test_string_hash_set()
{

    FcitxStringHashSet* sset = fcitx_string_hashset_parse("a,b,c,d", ',');
    assert(fcitx_dict_size(sset) == 4);
    assert(fcitx_string_hashset_contains(sset, "c"));
    assert(!fcitx_string_hashset_contains(sset, "e"));
    fcitx_string_hashset_remove(sset, "c");
    assert(!fcitx_string_hashset_contains(sset, "c"));
    fcitx_string_hashset_insert(sset, "e");
    assert(fcitx_string_hashset_contains(sset, "e"));

    /* uthash guarantee order, so we can sure about this */
    char* joined = fcitx_string_hashset_join(sset, ',');
    assert(strcmp(joined, "a,b,d,e") == 0);

    free(joined);

    fcitx_string_hashset_free(sset);
}

void test_string_map()
{
    FcitxStringMap* map = fcitx_string_map_new("a:true,b:false", ',');
    assert(fcitx_string_map_get(map, "a", false));
    assert(!fcitx_string_map_get(map, "b", false));
    assert(fcitx_string_map_get(map, "c", true));
    fcitx_string_map_set(map, "c", false);
    assert(!fcitx_string_map_get(map, "c", true));
    char* joined = fcitx_string_map_to_string(map, '\n');
    assert(strcmp(joined, "a:true\nb:false\nc:false") == 0);
    free(joined);

    fcitx_string_map_free(map);
}

int main()
{
    test_string();
    test_string_hash_set();
    test_string_map();
    return 0;
}
