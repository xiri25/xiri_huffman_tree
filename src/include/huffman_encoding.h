#ifndef HUFFMAN_ENCODING_H
#define HUFFMAN_ENCODING_H

#include <huffman_tree.h>

#include <stdint.h>

struct ht_dict {
    uint16_t code;
    uint8_t len;
};

void ht_dict_print_truncated(const struct ht_dict* dict, const uint16_t size);
void huffman_dict_create(const struct ht_tree* tree, struct ht_dict* dict);

#endif // !HUFFMAN_ENCODING_H
