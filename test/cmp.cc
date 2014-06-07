#include "fcitx-utils/radix.h"
#include "fcitx-utils/multiradix.h"
#include "fcitx-utils/utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <unistd.h>

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

int main()
{
    FcitxRadixTree* radix = fcitx_radix_tree_new(myfree, NULL);
    std::string key, value;
    while (std::cin >> key >> value) {
         fcitx_radix_tree_add(radix, key.c_str(), strdup(value.c_str()));
    }
    sleep(5);
    fcitx_radix_tree_free(radix);
    return 0;
}
