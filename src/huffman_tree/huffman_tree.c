#include <huffman_tree.h>
#include <file.h>
#include <frequencies.h>
#include <ASSERT.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool _is_root(const struct ht_node* node)
{
    if (node->parent_node == -1) return true;
    return false;
}

static bool _is_leaf(const struct ht_node* node)
{
    if ((node->left_node == -1) && (node->right_node == -1)) return true;
    return false;
}

/* TODO: check if the char is printable (or \n \0...) and print it */
void ht_node_print(const struct ht_tree* tree, const struct ht_node* node)
{
    char is_root[5] = {};
    if (_is_root(node)) {
        strcpy(is_root, "root\0");
    }
    if (_is_leaf(node)) {
        strcpy(is_root, "leaf\0");
    }
    printf("%p / %lu -> %d: %lu, p: (%d / %p), l: (%d / %p), r: (%d / %p), %s\n",
           node, ((uintptr_t)node - (uintptr_t)tree->tree) / sizeof(struct ht_node),
           node->c, node->weight,
           node->parent_node, &tree->tree[node->parent_node],
           node->left_node, &tree->tree[node->left_node],
           node->right_node, &tree->tree[node->right_node],
           is_root);
}

void ht_node_print_without_pointers(const struct ht_tree* tree, const struct ht_node* node)
{
    char is_root[5] = {};
    if (_is_root(node)) {
        strcpy(is_root, "root\0");
    }
    if (_is_leaf(node)) {
        strcpy(is_root, "leaf\0");
    }
    printf("%lu -> %d: %lu, p: (%d), l: (%d), r: (%d), %s\n",
           ((uintptr_t)node - (uintptr_t)tree->tree) / sizeof(struct ht_node),
           node->c, node->weight,
           node->parent_node,
           node->left_node,
           node->right_node,
           is_root);
}

void huffman_tree_calc_freq_from_file(const char* filepath, struct char_freq* sorted_freq_buffer, const uint16_t buffer_size)
{
    struct file_t file = {};
    file_open(&file, filepath);

    /* TODO: Dont read the entire file at once */
    const uint32_t chars_to_read = (uint32_t)file.size;
    uint8_t* file_buffer = malloc(sizeof(unsigned char) * chars_to_read);

    file_read_to_uint8_t_buffer(&file, file_buffer, chars_to_read);

    struct char_freq* frequencies = char_freq_buffer_alloc();
    char_freq_buffer_calculate(frequencies, file_buffer, chars_to_read);

    char_freq_buffer_sort(frequencies, sorted_freq_buffer, buffer_size, SORT_ASC);

    char_freq_buffer_free(frequencies);

    free(file_buffer);
    file_close(&file);
}

static uint16_t less_freq_node_idx(const struct ht_node* tree,
                                   const uint16_t tree_size)
{
    uint64_t smallest_weight = UINT64_MAX;
    uint16_t smallest_index = UINT16_MAX;
    uint16_t index = 0;

    /* TODO: Maybe a little cleanup, + meter las condiciones en el while if posible??? */
    while (index < tree_size) {
        if ((tree[index].weight < smallest_weight) &&
            (tree[index].parent_node == -1)) {
            smallest_weight = tree[index].weight;
            smallest_index = index;
        }
        index++;
    }

    /*
    printf("less_freq_node {weight = %lu}-> index = %u, weight = %lu\n",
           smallest_weight, smallest_index, tree[smallest_index].weight);
    */
    return smallest_index;
}

static uint16_t second_less_freq_node_idx(const struct ht_node* tree,
                                          const uint16_t tree_size,
                                          const uint16_t less_freq_node_idx)
{
    uint64_t second_smallest_weight = UINT64_MAX;
    uint16_t smallest_index = UINT16_MAX;
    uint16_t index = 0;

    /* TODO: Maybe a little cleanup, + meter las condiciones en el while if posible??? */
    while (index < tree_size) {
        if ((tree[index].weight < second_smallest_weight) &&
            (tree[index].parent_node == -1) &&
            (index != less_freq_node_idx)) {
            second_smallest_weight = tree[index].weight;
            smallest_index = index;
        }
        index++;
    }
    /*
    printf("second_less_freq_node {weight = %lu} -> index = %u, weight = %lu\n",
           second_smallest_weight, smallest_index, tree[smallest_index].weight);
    */
    return smallest_index;
}

struct ht_tree huffman_tree_create(const struct char_freq* sorted_freq,
                                   const size_t sorted_freq_len,
                                   struct ht_node* tree_buffer)
{
    uint16_t leaf_nodes = 0;
    for (uint64_t i = 0; i < sorted_freq_len; i++) {
        if (sorted_freq[i].freq == 0) {
            continue;
        } else {
            const struct ht_node node = {
                .c           = sorted_freq[i].c,
                .weight      = sorted_freq[i].freq,
                .parent_node = -1,
                .left_node   = -1,
                .right_node  = -1,
            };
            //printf("Leaf node: index = %u, weight = %lu\n", leaf_nodes, node.weight);
            tree_buffer[leaf_nodes] = node;
            leaf_nodes++;
        }
    }

    uint16_t nodes_wo_parent = leaf_nodes;
    uint16_t tree_size = leaf_nodes;
    while (nodes_wo_parent > 1) {

        const uint16_t first_index = less_freq_node_idx(tree_buffer, tree_size);
        const uint16_t second_index = second_less_freq_node_idx(tree_buffer, tree_size, first_index);

        struct ht_node* less_frequent_node = &tree_buffer[first_index];
        struct ht_node* second_less_frequent_node = &tree_buffer[second_index];

        const struct ht_node new_node = {
            .c           = (unsigned char)0,
            .weight      = less_frequent_node->weight + second_less_frequent_node->weight,
            .parent_node = -1,
            .left_node   = (int16_t)first_index,
            .right_node  = (int16_t)second_index,
        };

        /*
        printf("New node -> index = %lu, c = 0, weight = %lu, p_n = -1, l_n = %u, r_n = %u\n",
               j, new_node.weight, new_node.left_node, new_node.right_node);
        */

        tree_buffer[tree_size] = new_node;
        less_frequent_node->parent_node = (int16_t)tree_size;
        second_less_frequent_node->parent_node = (int16_t)tree_size;
        nodes_wo_parent -= 1; /* He creado un parent para dos nodes, pero el parent en si no tiene parent */
        tree_size += 1;
    }
    /*
     * NOTE: The tree can be as large as 2*(leaf_nodes) - 1
     * x + x/2 + x/4 + x/8 ..... + 1
    */
    /* The index of the no leaf nodes should not be greater that the size of the tree */
    ASSERT(tree_size <= sorted_freq_len * 2 - 1);

    const struct ht_tree tree = {
        .tree = tree_buffer,
        .root_idx = (uint16_t)tree_size - 1, /* El ultimo nodo es claramente el root_node */
        .node_count = (uint16_t)tree_size, /* TODO: los indices no necesitan ser uint64_t */
    };

    printf("the tree is %lu bytes\n", sizeof(struct ht_node) * tree_size);

    return tree;
}

/* Pass in the root, pre-order traversal (GePeTo lo dice) */
/* TODO: Arreglar este desastre */
void ht_tree_print(const struct ht_tree* tree, const struct ht_node* ht_node)
{
    if (ht_node == NULL) return;

    ht_node_print(tree, ht_node);

    if (ht_node->left_node == -1) return;
    if (ht_node->right_node == -1) return;

    ht_tree_print(tree, &tree->tree[ht_node->left_node]);
    ht_tree_print(tree, &tree->tree[ht_node->right_node]);
}

void ht_tree_print_without_pointers(const struct ht_tree* tree, const struct ht_node* ht_node)
{
    if (ht_node == NULL) return;

    ht_node_print_without_pointers(tree, ht_node);

    if (ht_node->left_node == -1) return;
    if (ht_node->right_node == -1) return;

    ht_tree_print_without_pointers(tree, &tree->tree[ht_node->left_node]);
    ht_tree_print_without_pointers(tree, &tree->tree[ht_node->right_node]);
}
