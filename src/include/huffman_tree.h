#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <frequencies.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * NOTE: Quizas quiero que sea una packed struct a la hora de serializar
 */
// struct __attribute__((packed)) ht_node {
struct ht_node {
    uint64_t weight;
    int16_t parent_node;
    int16_t left_node;
    int16_t right_node;
    uint8_t c;
};

struct ht_tree {
    struct ht_node* tree;
    uint16_t root_idx;
    uint16_t node_count;
};

void huffman_tree_calc_freq_from_file(const char* filepath, struct char_freq* sorted_freq_buffer, const uint16_t buffer_size);
uint64_t ht_nodes_without_parent(const struct ht_node* ht_tree, const uint64_t freq_len);
struct ht_tree huffman_tree_create(const struct char_freq* sorted_freq,
                                   const size_t sorted_freq_len,
                                   struct ht_node* tree_buffer);
void ht_tree_print(const struct ht_tree* tree, const struct ht_node* ht_node);

#endif // !HUFFMAN_TREE_H
