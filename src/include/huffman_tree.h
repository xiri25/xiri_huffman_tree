#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <frequencies.h>

#include <stdbool.h>

// struct __attribute__((packed)) huffman_tree_node {
struct ht_node {
    uint64_t weight;
    struct ht_node* parent_node;
    struct ht_node* left_node;
    struct ht_node* right_node;
    uint8_t c;
    bool is_root;
    bool is_leaf;
};

void huffman_tree_calc_freq_from_file(const char* filepath, struct char_freq* sorted_freq_buffer, const uint16_t buffer_size);
void print_ht_node(const struct ht_node* node);

#endif // !HUFFMAN_TREE_H
