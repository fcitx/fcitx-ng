/* Copyright (c) 2007-2010 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include "fcitx/fcitx.h"
#include "utarray.h"
#include "sort_common.h"

/* damn it, freebsd and linux don't have the same interface for qsort_r,
 * and old glibc don't have qsort_r, force it to use third party version.
 */

FCITX_EXPORT_API
void fcitx_qsort_r(void *base_, size_t nmemb, size_t size, int (*compar)(const void *, const void *, void *), void *thunk)
{
    char *base = (char *) base_;
    if (nmemb < 10) { /* use O(nmemb^2) algorithm for small enough nmemb */
        insertion_sort(base, nmemb, size, compar, thunk);
    } else {
        size_t i, pivot, npart;
        /* pick median of first/middle/last elements as pivot */
        {
            const char *a = base, *b = base + (nmemb / 2) * size,
                        *c = base + (nmemb - 1) * size;
            pivot = compar(a, b, thunk) < 0
                    ? (compar(b, c, thunk) < 0 ? nmemb / 2 :
                       (compar(a, c, thunk) < 0 ? nmemb - 1 : 0))
                        : (compar(a, c, thunk) < 0 ? 0 :
                           (compar(b, c, thunk) < 0 ? nmemb - 1 : nmemb / 2));
        }
        /* partition array */
        swap(base + pivot * size, base + (nmemb - 1) * size, size);
        pivot = (nmemb - 1) * size;
        for (i = npart = 0; i < nmemb - 1; ++i)
            if (compar(base + i * size, base + pivot, thunk) <= 0)
                swap(base + i * size, base + (npart++)*size, size);
        swap(base + npart * size, base + pivot, size);
        /* recursive sort of two partitions */
        fcitx_qsort_r(base, npart, size, compar, thunk);
        npart++; /* don't need to sort pivot */
        fcitx_qsort_r(base + npart * size, nmemb - npart, size, compar, thunk);
    }
}
