#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <frequencies.h>

#include <stdbool.h>
#include <stddef.h>

// struct __attribute__((packed)) huffman_tree_node {
struct ht_node {
    /*
     * TODO: Change pointer por int16_t indexes (-1 for NULL)
     * Get rid of is_root and is_leaf, maybe create methods to check
     */
    uint64_t weight;
    struct ht_node* parent_node;
    struct ht_node* left_node;
    struct ht_node* right_node;
    uint8_t c;
    bool is_root;
    bool is_leaf;
};

struct ht_tree {
    struct ht_node* tree;
    struct ht_node* root;
    uint16_t node_count;
};

void huffman_tree_calc_freq_from_file(const char* filepath, struct char_freq* sorted_freq_buffer, const uint16_t buffer_size);
void ht_node_print(const struct ht_node* node);
uint64_t ht_nodes_without_parent(const struct ht_node* ht_tree, const uint64_t freq_len);
struct ht_tree huffman_tree_create(const struct char_freq* sorted_freq,
                                   const size_t sorted_freq_len,
                                   struct ht_node* tree_buffer);
void ht_tree_print(const struct ht_node* ht_node);

#endif // !HUFFMAN_TREE_H
