#include <assert.h>
#include "fcitx-utils/utils.h"

int cmp(const char** a, const char** b)
{
    return strcmp(*a, *b);
}

int main()
{
    UT_array array;
    utarray_init(&array, &ut_int_icd);
    int test[] = {3, 1, 2};
    int test2[] = {1, 2, 3};
    FCITX_UNUSED(test);
    FCITX_UNUSED(test2);
    int i;
    i = 1;
    utarray_push_back(&array, &i);
    i = 2;
    utarray_push_back(&array, &i);
    i = 3;
    utarray_push_back(&array, &i);
    utarray_move(&array, 2, 0);
    {
        utarray_foreach(p, &array, int) {
            assert(*p == test[utarray_eltidx(&array, p)]);
        }
    }
    utarray_move(&array, 0, 2);

    {
        utarray_foreach(p, &array, int) {
            assert(*p == test2[utarray_eltidx(&array, p)]);
        }
    }
    utarray_done(&array);

    FcitxPtrArray* ptrArray = fcitx_ptr_array_new(free);
    fcitx_ptr_array_append(ptrArray, fcitx_utils_strdup("E"));
    fcitx_ptr_array_append(ptrArray, fcitx_utils_strdup("D"));
    fcitx_ptr_array_append(ptrArray, fcitx_utils_strdup("A"));
    fcitx_ptr_array_append(ptrArray, fcitx_utils_strdup("C"));
    fcitx_ptr_array_append(ptrArray, fcitx_utils_strdup("B"));

    assert(fcitx_ptr_array_size(ptrArray) == 5);

    fcitx_ptr_array_remove(ptrArray, 0, NULL);
    assert(fcitx_ptr_array_size(ptrArray) == 4);

    fcitx_ptr_array_remove_fast(ptrArray, 2, NULL);
    assert(fcitx_ptr_array_size(ptrArray) == 3);

    fcitx_ptr_array_insert(ptrArray, fcitx_utils_strdup("C"), 1);
    assert(fcitx_ptr_array_size(ptrArray) == 4);

    fcitx_ptr_array_sort(ptrArray, (FcitxCompareFunc) cmp);

    assert(strcmp(fcitx_ptr_array_index(ptrArray, 0, char*), "A") == 0);
    assert(strcmp(fcitx_ptr_array_index(ptrArray, 1, char*), "B") == 0);
    assert(strcmp(fcitx_ptr_array_index(ptrArray, 2, char*), "C") == 0);
    assert(strcmp(fcitx_ptr_array_index(ptrArray, 3, char*), "D") == 0);

    fcitx_ptr_array_clear(ptrArray);
    assert(fcitx_ptr_array_size(ptrArray) == 0);

    fcitx_ptr_array_free(ptrArray);

    return 0;
}
