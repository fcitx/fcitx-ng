#include <assert.h>
#include "fcitx-utils/memory-pool.h"

int main()
{
    FcitxMemoryPool* pool = fcitx_memory_pool_new();
    fcitx_memory_pool_alloc(pool, 10);
    fcitx_memory_pool_clear(pool);

    fcitx_memory_pool_alloc(pool, 10);
    fcitx_memory_pool_alloc(pool, 10);
    fcitx_memory_pool_free(pool);

    return 0;
}
