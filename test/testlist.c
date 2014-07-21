#include "fcitx-utils/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

struct Data
{
    int a;
    FcitxListHead list;
};

int cmp(const void* a, const void* b)
{
    const struct Data* da = a, *db = b;
    return db->a - da->a;
}

int main()
{
    FcitxListHead head;
    fcitx_list_init(&head);

#define N_DATA 10

    struct Data data[N_DATA];
    for(size_t i = 0; i < FCITX_ARRAY_SIZE(data); i ++) {
        data[i].a = i;
        fcitx_list_append(&data[i].list, &head);
    }

    struct Data *d2, *tmpd;
    FcitxListHead* item2, *tmpitem;

    int j = 0;
    fcitx_list_foreach(item, &head) {
        struct Data* d = fcitx_container_of(item, struct Data, list);
        assert(d->a == j);
        j ++;
    }
    assert(j == N_DATA);
    j = 0;
    fcitx_list_foreach_safe(item, &head) {
        struct Data* d = fcitx_container_of(item, struct Data, list);
        assert(d->a == j);
        j ++;
    }
    assert(j == N_DATA);
    j = 0;
    fcitx_list_foreach_nl(item2, &head) {
        struct Data* d = fcitx_container_of(item2, struct Data, list);
        assert(d->a == j);
        j ++;
    }
    assert(j == N_DATA);
    j = 0;
    fcitx_list_foreach_safe_nl(item2, tmpitem, &head) {
        struct Data* d = fcitx_container_of(item2, struct Data, list);
        assert(d->a == j);
        j ++;
    }
    assert(j == N_DATA);

    j = 0;
    fcitx_list_entry_foreach(d, struct Data, &head, list) {
        assert(d->a == j);
        j ++;
    }
    assert(j == N_DATA);

    j = 0;
    fcitx_list_entry_foreach_safe(d, struct Data, &head, list) {
        assert(d->a == j);
        j ++;
    }
    assert(j == N_DATA);
    j = 0;
    fcitx_list_entry_foreach_nl(d2, struct Data, &head, list) {
        assert(d2->a == j);
        j ++;
    }
    assert(j == N_DATA);
    
    fcitx_list_sort(&head, offsetof(struct Data, list), cmp);
    j = 0;
    fcitx_list_entry_foreach_nl(d2, struct Data, &head, list) {
        assert(d2->a == N_DATA - j - 1);
        j ++;
    }
    assert(j == N_DATA);

    j = 0;
    fcitx_list_entry_foreach_safe_nl(d2, tmpd, struct Data, &head, list) {
        fcitx_list_remove(&d2->list);
        assert(d2->a == N_DATA - j - 1);
        j ++;
    }
    assert(j == N_DATA);
    assert(fcitx_list_is_empty(&head));

    return 0;
}
