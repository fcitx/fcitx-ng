#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include "fcitx-utils/utils.h"

typedef struct {
    int a;
    int b;
} SortItem;

SortItem array[] = {
    {3, 4},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
};

SortItem msort_array[] = {
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {2, 4},
    {2, 3},
    {3, 4},
};


int cmp(const void* a, const void* b, void* thunk)
{
    return ((SortItem*)a)->a - ((SortItem*)b)->a;
}

int intcmp(const void* a, const void* b, void* thunk)
{
    if (thunk) {
        return (*((int*)b)) - (*((int*)a));
    }
    return (*((int*)a)) - (*((int*)b));
}

int main()
{
    /* stable test */
    fcitx_msort_r(array, FCITX_ARRAY_SIZE(array), sizeof(array[0]), cmp, NULL);

    for (size_t i = 0; i < FCITX_ARRAY_SIZE(array); i ++) {
        assert(array[i].a == msort_array[i].a);
        assert(array[i].b == msort_array[i].b);
    }

    srand(time(NULL));

    {
        /* qsort_test */
        int qsort_intarray[1024];
        for (size_t i = 0; i <FCITX_ARRAY_SIZE(qsort_intarray); i ++) {
            qsort_intarray[i] = rand();
        }
        fcitx_qsort_r(qsort_intarray, FCITX_ARRAY_SIZE(qsort_intarray), sizeof(qsort_intarray[0]), intcmp, NULL);

        for (size_t i = 0; i < FCITX_ARRAY_SIZE(qsort_intarray) - 1; i ++) {
            assert(qsort_intarray[i] <= qsort_intarray[i + 1]);
        }

        /* msort_test */
        fcitx_msort_r(qsort_intarray, FCITX_ARRAY_SIZE(qsort_intarray), sizeof(qsort_intarray[0]), intcmp, (void*) 0x1);

        for (size_t i = 0; i < FCITX_ARRAY_SIZE(qsort_intarray) - 1; i ++) {
            assert(qsort_intarray[i] >= qsort_intarray[i + 1]);
        }
    }

    {
        /* qsort_test */
        int qsort_intarray[6];
        for (size_t i = 0; i <FCITX_ARRAY_SIZE(qsort_intarray); i ++) {
            qsort_intarray[i] = rand();
        }
        fcitx_qsort_r(qsort_intarray, FCITX_ARRAY_SIZE(qsort_intarray), sizeof(qsort_intarray[0]), intcmp, (void*) 0x1);

        for (size_t i = 0; i < FCITX_ARRAY_SIZE(qsort_intarray) - 1; i ++) {
            assert(qsort_intarray[i] >= qsort_intarray[i + 1]);
        }

        /* msort_test */
        fcitx_msort_r(qsort_intarray, FCITX_ARRAY_SIZE(qsort_intarray), sizeof(qsort_intarray[0]), intcmp, NULL);

        for (size_t i = 0; i < FCITX_ARRAY_SIZE(qsort_intarray) - 1; i ++) {
            assert(qsort_intarray[i] <= qsort_intarray[i + 1]);
        }
    }

    return 0;
}
