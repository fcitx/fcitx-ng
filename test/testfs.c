#include "fcitx-utils/utils.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define TEST_PATH(PATHSTR, EXPECT) do { \
        char pathstr[] = PATHSTR; \
        char bufstr[FCITX_ARRAY_SIZE(pathstr)]; \
        size_t result = fcitx_utils_clean_path(pathstr, bufstr); \
        assert(strlen(bufstr) == result); \
        assert(strcmp(bufstr, EXPECT) == 0); \
    } while(0);

int main()
{
    TEST_PATH("/a", "/a");
    TEST_PATH("/a/b", "/a/b");
    TEST_PATH("a/b", "a/b");
    TEST_PATH("///a/b", "///a/b");
    TEST_PATH("///", "///");
    TEST_PATH("///a/..", "///");
    TEST_PATH("///a/./b", "///a/b");
    TEST_PATH("./././.", "");
    TEST_PATH("", "");
    TEST_PATH("../././.", "../");
    TEST_PATH("../././..", "../..");
    TEST_PATH(".././../.", "../../");
    TEST_PATH("///.././../.", "///../../");
    TEST_PATH("///a/./../c", "///c");
    TEST_PATH("./../a/../c/b", "../c/b");
    TEST_PATH("./.../a/../c/b", ".../c/b");

    assert(!fcitx_utils_isdir("a"));
    assert(!fcitx_utils_isdir("a/b"));
    assert(!fcitx_utils_isdir("a/b/c"));
    assert(fcitx_utils_make_path("a/b/c"));
    assert(fcitx_utils_make_path("///"));
    assert(fcitx_utils_make_path("a/b/c"));
    assert(fcitx_utils_make_path("a/b/d"));
    assert(fcitx_utils_make_path("a/b"));
    assert(fcitx_utils_make_path("a"));
    assert(fcitx_utils_make_path(""));
    assert(fcitx_utils_isdir("a"));
    assert(fcitx_utils_isdir("a/b"));
    assert(fcitx_utils_isdir("a/b/c"));
    assert(fcitx_utils_isdir("a/b/d"));
    assert(rmdir("a/b/c") == 0);
    assert(rmdir("a/b/d") == 0);
    assert(rmdir("a/b") == 0);
    assert(rmdir("a") == 0);
    return 0;
}
