#ifndef MULTIRADIX_H
#define MULTIRADIX_H

#include <fcitx-utils/types.h>
#include <fcitx-utils/list.h>
#include <fcitx-utils/radix.h>

typedef struct _FcitxMultiRadixTree FcitxMultiRadixTree;

FcitxMultiRadixTree* fcitx_multi_radix_tree_new(size_t off, FcitxClosureFunc destroyNotify, void* data);
void fcitx_multi_radix_tree_free(FcitxMultiRadixTree* multiRadix);
void fcitx_multi_radix_tree_add(FcitxMultiRadixTree* multiRadix, const char* key, void* leaf);
void fcitx_multi_radix_tree_remove(FcitxMultiRadixTree* multiRadix, char* key, void* item);
size_t fcitx_multi_radix_tree_size(FcitxMultiRadixTree* multiRadix);
void fcitx_multi_radix_tree_foreach(FcitxMultiRadixTree* multiRadix, const char* key, FcitxRadixTreeForeachCallback callback, void* data);
FcitxListHead* fcitx_multi_radix_tree_exact_match(FcitxMultiRadixTree* multiRadix, const char* key);

#endif // MULTIRADIX_H