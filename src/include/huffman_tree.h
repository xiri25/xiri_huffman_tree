#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <frequencies.h>
#include <file.h>

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

void huffman_tree_calc_freq_from_file(const struct file_raw_t* file,
                                      struct char_freq* sorted_freq_buffer,
                                      const uint16_t buffer_size);
struct ht_tree huffman_tree_create(const struct char_freq* sorted_freq,
                                   const size_t sorted_freq_len,
                                   struct ht_node* tree_buffer);
void ht_tree_print(const struct ht_tree* tree, const bool print_pointers);
static bool ht_node_is_root(const struct ht_node* node)
{
    if (node->parent_node == -1) return true;
    return false;
}

static bool ht_node_is_leaf(const struct ht_node* node)
{
    if ((node->left_node == -1) && (node->right_node == -1)) return true;
    return false;
}

#endif // !HUFFMAN_TREE_H
