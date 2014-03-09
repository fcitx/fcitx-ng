#include "fcitx-utils/radix.h"
#include "fcitx-utils/multiradix.h"
#include "fcitx-utils/utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TEST_SIZE(prefix, num) do { \
    size_t c; \
    c = 0; \
    fcitx_radix_tree_foreach(radix, prefix, count, &c); \
    assert(c == num); \
} while(0)

void myfree(void* a, void* b)
{
    free(a);
}

void print(const char* key, void* data, void* userData)
{
    char* s = (char*) data;
    printf("%s: %s\n", key, s);
}

void count(const char* key, void* data, void* userData)
{
    size_t* s = (size_t*) userData;
    (*s)++;
}

void test_radix()
{
    FcitxRadixTree* radix = fcitx_radix_tree_new(myfree, NULL);
    assert(fcitx_radix_tree_add(radix, "aa", strdup("1")));
    assert(fcitx_radix_tree_add(radix, "ab", strdup("2")));
    assert(fcitx_radix_tree_add(radix, "abcd", strdup("2")));
    assert(fcitx_radix_tree_add(radix, "ac", strdup("2")));
    assert(fcitx_radix_tree_add(radix, "a", strdup("3")));
    assert(fcitx_radix_tree_add(radix, "b", strdup("4")));
    assert(fcitx_radix_tree_add(radix, "aaa", strdup("5")));
    assert(7 == fcitx_radix_tree_size(radix));
    TEST_SIZE(NULL, 7);
    TEST_SIZE("a", 6);
    TEST_SIZE("aa", 2);
    TEST_SIZE("abc", 1);
    
    fcitx_radix_tree_foreach(radix, NULL, print, NULL);
    assert(!fcitx_radix_tree_add(radix, "aa", "5"));
    assert(!fcitx_radix_tree_add(radix, "ab", "5"));
    assert(!fcitx_radix_tree_add(radix, "a", "5"));
    assert(!fcitx_radix_tree_add(radix, "b", "5"));
    assert(fcitx_radix_tree_remove(radix, "a"));
    fcitx_radix_tree_foreach(radix, NULL, print, NULL);
    assert(fcitx_radix_tree_remove(radix, "aa"));
    assert(fcitx_radix_tree_remove(radix, "ab"));
    assert(fcitx_radix_tree_remove(radix, "ac"));
    assert(fcitx_radix_tree_remove(radix, "aaa"));
    assert(fcitx_radix_tree_remove(radix, "b"));
    assert(!fcitx_radix_tree_remove(radix, "b"));
    assert(fcitx_radix_tree_add(radix, "b", strdup("5")));
    TEST_SIZE(NULL, 2);
    TEST_SIZE("a", 1);
    TEST_SIZE("b", 1);
    fcitx_radix_tree_foreach(radix, NULL, print, NULL);
    fcitx_radix_tree_foreach(radix, "a", print, NULL);
    fcitx_radix_tree_free(radix);
}

typedef struct Data
{
    int a;
    FcitxListHead list;
} Data;

void test_multiradix()
{
    FcitxMultiRadixTree* multiRadix = fcitx_multi_radix_tree_new(offsetof(Data, list), fcitx_utils_closure_free, NULL);
    for(int i = 0; i < 10; i ++) {
        char key[3] = "00";
        Data* d = fcitx_utils_new(Data);
        d->a = i;
        key[1] += i;
        fcitx_multi_radix_tree_add(multiRadix, key, d);
    }
    
    assert(fcitx_multi_radix_tree_size(multiRadix) == 10);
    
    fcitx_multi_radix_tree_free(multiRadix);
}

int main()
{
    test_radix();
    test_multiradix();
    return 0;
}
