/*
  Copyright (c) 2008-2010, Troy D. Hanson   http://uthash.sourceforge.net
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* a dynamic array implementation using macros
 * see http://uthash.sourceforge.net/utarray
 */
#ifndef _FCITX_UTILS_UTARRAY_H_
#define _FCITX_UTILS_UTARRAY_H_

#if !defined (_FCITX_UTILS_H_INSIDE_)
#error "Only <fcitx-utils/utils.h> can be included directly."
#endif

#include <stddef.h>  /* size_t */
#include <string.h>  /* memset, etc */
#include <stdlib.h>
#include "macro.h"

typedef void (ctor_f)(void *dst, const void *src);
typedef void (dtor_f)(void *elt);
typedef void (init_f)(void *elt);
typedef struct {
    size_t sz;
    init_f *init;
    ctor_f *copy;
    dtor_f *dtor;
} UT_icd;

typedef struct {
    size_t i, n; /* i: index of next available slot, n: num slots */
    const UT_icd *icd; /* initializer, copy and destructor functions */
    void *d;     /* n slots of size icd->sz*/
} UT_array;

static _FCITX_ALWAYS_INLINE_ void utarray_init(UT_array* a, const UT_icd* icd)
{
    memset(a, 0, sizeof(UT_array));
    (a)->icd = icd;
}


static _FCITX_ALWAYS_INLINE_ void* _utarray_eltptr(const UT_array* a, size_t j)
{
    return ((char*)((a)->d + ((a)->icd->sz * (j))));
}

static _FCITX_ALWAYS_INLINE_ void* utarray_eltptr(UT_array* a, size_t j)
{
    return (((j) < (a)->i) ? _utarray_eltptr(a, j) : NULL);
}

static _FCITX_ALWAYS_INLINE_ void utarray_done(UT_array* a)
{
    if ((a)->n) {
        if ((a)->icd->dtor) {
            size_t _ut_i;
            for(_ut_i = 0;_ut_i < (a)->i;_ut_i++) {
                (a)->icd->dtor(utarray_eltptr(a, _ut_i));
            }
        }
        free((a)->d);
    }
    (a)->n = 0;
}

static _FCITX_ALWAYS_INLINE_ void* utarray_steal(UT_array* a)
{
    if (!(a)->n) {
        return NULL;
    }
    void* p = (a)->d;
    (a)->d = NULL;
    (a)->n = (a)->i = 0;
    return p;
}

static _FCITX_ALWAYS_INLINE_ UT_array* utarray_new (const UT_icd* icd)
{
    UT_array* a = (UT_array*) malloc(sizeof(UT_array));
    utarray_init(a, icd);
    return a;
}

static _FCITX_ALWAYS_INLINE_ void utarray_free(UT_array* a)
{
    utarray_done(a);
    free(a);
}

static _FCITX_ALWAYS_INLINE_ void utarray_reserve(UT_array* a, size_t by)
{
    if (((a)->i + by) > ((a)->n)) {
        while(((a)->i + by) > ((a)->n)) {
            (a)->n = ((a)->n ? (2 * (a)->n) : 8);
        }
        if (!((a)->d = (char*)realloc((a)->d, (a)->n * (a)->icd->sz))) {
            abort();
        }
    }
}

static _FCITX_ALWAYS_INLINE_ void utarray_push_back(UT_array* a, void* p) {
    utarray_reserve(a, 1);
    if ((a)->icd->copy) {
        (a)->icd->copy(_utarray_eltptr(a, (a)->i++), p);
    } else {
        memcpy(_utarray_eltptr(a, (a)->i++), p, (a)->icd->sz);
    };
}

static _FCITX_ALWAYS_INLINE_ void utarray_pop_back(UT_array* a, void* p) {
    if ((a)->icd->dtor) {
        (a)->icd->dtor(_utarray_eltptr(a, --((a)->i)));
    } else {
        (a)->i--;
    }
}

static _FCITX_ALWAYS_INLINE_ void utarray_extend_back(UT_array* a) {
    utarray_reserve(a, 1);
    if ((a)->icd->init) {
        (a)->icd->init(_utarray_eltptr(a, (a)->i));
    } else {
        memset(_utarray_eltptr(a, (a)->i), 0, (a)->icd->sz);
    }
    (a)->i++;
}

#define utarray_len(a) ((a)->i)

static _FCITX_ALWAYS_INLINE_ void utarray_push_front(UT_array* a, void* p)
{
    utarray_reserve(a, 1);
    if (0 < (a)->i) {
        memmove(_utarray_eltptr(a, 1), _utarray_eltptr(a, 0),
                ((a)->i) * ((a)->icd->sz));
    }
    if ((a)->icd->copy) {
        (a)->icd->copy(_utarray_eltptr(a, 0), p);
    } else {
        memcpy(_utarray_eltptr(a, 0), p, (a)->icd->sz);
    };
    (a)->i++;
}

static _FCITX_ALWAYS_INLINE_ void utarray_insert(UT_array* a, void* p, size_t j)
{
    utarray_reserve(a, 1);
    if (j > (a)->i) {
        return;
    }
    if ((j) < (a)->i) {
        memmove(_utarray_eltptr(a, (j) + 1), _utarray_eltptr(a, j),
                ((a)->i - (j)) * ((a)->icd->sz));
    }
    if ((a)->icd->copy) {
        (a)->icd->copy(_utarray_eltptr(a, j), p);
    } else {
        memcpy(_utarray_eltptr(a, j), p, (a)->icd->sz);
    };
    (a)->i++;
}

static _FCITX_ALWAYS_INLINE_ void utarray_move(UT_array* a, size_t f, size_t t) {
    if (f >= (a)->i) {
        return;
    }
    if (t >= (a)->i) {
        return;
    }
    if (f == t) {
        return;
    }
    void *_temp = malloc((a)->icd->sz);
    if (f > t) {
        memcpy(_temp, _utarray_eltptr(a, f), (a)->icd->sz);
        memmove(_utarray_eltptr(a, t + 1), _utarray_eltptr(a, t),
                (a)->icd->sz * (f - (t)));
        memcpy(_utarray_eltptr(a, t), _temp, (a)->icd->sz);
    } else {
        memcpy(_temp, _utarray_eltptr(a, f), (a)->icd->sz);
        memmove(_utarray_eltptr(a, f), _utarray_eltptr(a, f + 1),
                (a)->icd->sz * (t - (f)));
        memcpy(_utarray_eltptr(a, t), _temp, (a)->icd->sz);
    }
    free(_temp);
}

static _FCITX_ALWAYS_INLINE_ void utarray_inserta(UT_array* a, UT_array* w, size_t j)
{
    if (utarray_len(w) == 0) {
        return;
    }
    if (j > (a)->i) {
        return;
    }
    utarray_reserve(a, utarray_len(w));
    if ((j) < (a)->i) {
        memmove(_utarray_eltptr(a, (j) + utarray_len(w)),
                _utarray_eltptr(a, j),
                ((a)->i - (j)) * ((a)->icd->sz));
    }
    if ((a)->icd->copy) {
        size_t _ut_i;
        for(_ut_i = 0;_ut_i < (w)->i;_ut_i++) {
            (a)->icd->copy(_utarray_eltptr(a, j + _ut_i),
                            _utarray_eltptr(w, _ut_i));
        }
    } else {
        memcpy(_utarray_eltptr(a, j), _utarray_eltptr(w, 0),
                utarray_len(w) * ((a)->icd->sz));
    }
    (a)->i += utarray_len(w);
}

static _FCITX_ALWAYS_INLINE_ void utarray_resize(UT_array* dst, size_t num)
{
    size_t _ut_i;
    if ((dst)->i > (size_t)(num)) {
        if ((dst)->icd->dtor) {
            for (_ut_i = num;_ut_i < (dst)->i;_ut_i++) {
                (dst)->icd->dtor(utarray_eltptr(dst, _ut_i));
            }
        }
    } else if ((dst)->i < (size_t)(num)) {
        utarray_reserve(dst, num - (dst)->i);
        if ((dst)->icd->init) {
            for (_ut_i = (dst)->i;_ut_i < num;_ut_i++) {
                (dst)->icd->init(utarray_eltptr(dst, _ut_i));
            }
        } else {
            memset(_utarray_eltptr(dst, (dst)->i), 0,
                    (dst)->icd->sz * (num - (dst)->i));
        }
    }
    (dst)->i = num;
}

#define utarray_concat(dst, src) utarray_inserta(dst, src, utarray_len(dst))

static _FCITX_ALWAYS_INLINE_ void utarray_erase(UT_array* a, size_t pos, size_t len) {
    if ((a)->icd->dtor) {
        size_t _ut_i;
        for(_ut_i = 0;_ut_i < len;_ut_i++) {
            (a)->icd->dtor(utarray_eltptr(a, pos + _ut_i));
        }
    }
    if ((a)->i > (pos + len)) {
        memmove(_utarray_eltptr(a, pos), _utarray_eltptr(a, pos + len),
                (((a)->i) - (pos + len)) * ((a)->icd->sz));
    }
    (a)->i -= (len);
}

/**
 * this actually steal the content away, doesn't destroy the content
 * so the name doesn't actually accurate, use it carefully.
 */
static _FCITX_ALWAYS_INLINE_ void utarray_remove_quick(UT_array* a, size_t pos)
{
    if ((a)->i - 1 != (pos))
        memcpy(_utarray_eltptr(a, pos), _utarray_eltptr(a, (a)->i - 1),
                (a)->icd->sz);
    (a)->i--;
}

/**
 * this is the "real" remove_quick with destroy,
 */
static _FCITX_ALWAYS_INLINE_ void utarray_remove_quick_full(UT_array* a, size_t pos)
{
    if ((a)->icd->dtor) {
        (a)->icd->dtor(utarray_eltptr(a, pos));
    }
    if ((a)->i - 1 != (pos))
        memcpy(_utarray_eltptr(a, pos), _utarray_eltptr(a, (a)->i - 1),
                (a)->icd->sz);
    (a)->i--;
}

static _FCITX_ALWAYS_INLINE_ void utarray_clear(UT_array* a) {
    if ((a)->i > 0) {
        if ((a)->icd->dtor) {
            size_t _ut_i;
            for(_ut_i = 0;_ut_i < (a)->i;_ut_i++) {
                (a)->icd->dtor(utarray_eltptr(a, _ut_i));
            }
        }
        (a)->i = 0;
    }
}

#define utarray_sort(a, cmp) qsort((a)->d, (a)->i, (a)->icd->sz, cmp)

static _FCITX_ALWAYS_INLINE_ void utarray_sort_range(UT_array* a, int (*cmp)(const void*, const void*), size_t from, size_t to)
{
    if ((from) >= (to) || (from) >= (a)->i || (to) > (a)->i) {
        return;
    }
    qsort(_utarray_eltptr(a, from), (to) - (from), (a)->icd->sz, cmp);
}

void fcitx_qsort_r(void *base_, size_t nmemb, size_t size,
                   int (*compar)(const void *, const void *, void *),
                   void *thunk);
void fcitx_msort_r(void *base_, size_t nmemb, size_t size,
                   int (*compar)(const void *, const void *, void *),
                   void *thunk);

#define utarray_sort_r(a, cmp, arg) fcitx_qsort_r((a)->d, (a)->i, (a)->icd->sz, cmp, arg)

#define utarray_msort_r(a, cmp, arg) fcitx_msort_r((a)->d, (a)->i, (a)->icd->sz, cmp, arg)

static _FCITX_ALWAYS_INLINE_ int utarray_eltidx(UT_array* a, void* e)
{
    return e >= a->d ?
           (((char*) e - (char*)a->d) / (int) a->icd->sz) : -1;
}

static _FCITX_ALWAYS_INLINE_ void* utarray_front(UT_array* a) {
    return (a->i ? (_utarray_eltptr(a, 0)) : NULL);
}

static _FCITX_ALWAYS_INLINE_ void* utarray_back(UT_array* a) {
    return (((a)->i) ? (_utarray_eltptr(a, (a)->i - 1)) : NULL);
}

static _FCITX_ALWAYS_INLINE_ void* utarray_next(UT_array* a, void* e)
{
    return (e == NULL ? utarray_front(a) :
            (((int)(a->i) > (utarray_eltidx(a, e) + 1)) ?
            _utarray_eltptr(a, utarray_eltidx(a, e) + 1) :
            NULL));
}
static _FCITX_ALWAYS_INLINE_ void*
utarray_prev(UT_array *a, void *e)
{
    if (!e)
        return utarray_back(a);
    int idx = utarray_eltidx(a, e) - 1;
    if (idx < 0)
        return NULL;
    return _utarray_eltptr(a, idx);
}

static const UT_icd ut_int_icd _FCITX_UNUSED_ = {
    sizeof(int), NULL, NULL, NULL
};

#define utarray_custom_bsearch(key, a, acc, cmp) fcitx_utils_custom_bsearch(key, (a)->d, (a)->i, (a)->icd->sz, acc, cmp)

#define utarray_foreach(key, array, type) \
    for (type *key = (type*)utarray_front(array);key; \
         key = (type*)utarray_next((array), key))

#define utarray_foreach_nl(key, array, type) \
    for (key = (type*)utarray_front(array);key; \
         key = (type*)utarray_next((array), key))

static _FCITX_ALWAYS_INLINE_ UT_array*
utarray_clone(const UT_array *from)
{
    UT_array *to = utarray_new(from->icd);
    if (utarray_len(from) == 0)
        return to;
    utarray_reserve(to, utarray_len(from));
    if (to->icd->copy) {
        size_t i;
        for(i = 0;i < from->i;i++) {
            to->icd->copy(_utarray_eltptr(to, i), _utarray_eltptr(from, i));
        }
    } else {
        memcpy(_utarray_eltptr(to, 0), _utarray_eltptr(from, 0),
               utarray_len(from) * (to->icd->sz));
    }
    to->i = utarray_len(from);
    return to;
}

#endif /* UTARRAY_H */

// kate: indent-mode cstyle; space-indent on; indent-width 0;
