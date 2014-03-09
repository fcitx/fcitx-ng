#ifndef FCITX_UTILS_RADIX_H
#define FCITX_UTILS_RADIX_H

#include <stddef.h>
#include <fcitx-utils/macro.h>
#include <fcitx-utils/types.h>

FCITX_DECL_BEGIN

typedef void (*FcitxRadixTreeForeachCallback)(const char* key, void* data, void* userData);

typedef struct _FcitxRadixTree FcitxRadixTree;

FcitxRadixTree* fcitx_radix_tree_new(FcitxClosureFunc destroyNotify, void* data);
void fcitx_radix_tree_free(FcitxRadixTree* radix);
boolean fcitx_radix_tree_add(FcitxRadixTree* radix, const char* key, void* leaf);
boolean fcitx_radix_tree_remove(FcitxRadixTree* radix, const char* key);
size_t fcitx_radix_tree_size(FcitxRadixTree* radix);

/**
 * enumerate all key with same prefix
 * 
 * @param radix tree
 * @param key key
 * @param callback callback
 * @param data user data
 * @return void
 */
void fcitx_radix_tree_foreach(FcitxRadixTree* radix, const char* key, FcitxRadixTreeForeachCallback callback, void* data);
void* fcitx_radix_tree_exact_match(FcitxRadixTree* radix, const char* key);

FCITX_DECL_END

#endif // FCITX_UTILS_RADIX_H