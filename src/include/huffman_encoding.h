#ifndef HUFFMAN_ENCODING_H
#define HUFFMAN_ENCODING_H

#include <huffman_tree.h>

#include <stdint.h>

struct ht_dict {
    uint16_t code;
    uint8_t len;
};

void build_huffman_dict_recursive(const struct ht_tree* tree,
                                  const struct ht_node* node,
                                  struct ht_dict* dict,
                                  struct ht_dict current_code);
void ht_dict_print_truncated(const struct ht_dict* dict, const uint16_t size);

#endif // !HUFFMAN_ENCODING_H
