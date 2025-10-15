#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include <frequencies.h>
#include <huffman_tree.h>
#include <memory/arena.h>

#define PRINT_DEBUG(x) printf(x);
// #define PRINT_DEBUG(x)

void print_bits_form_uint8_t(const uint8_t byte)
{
    for (uint32_t i = 0; i < 8; i++) {
        printf("%d", (byte >> (7 - i)) & 1);
    }
}

/* No se me podia ocurrir una idea peor (creo) */
uint64_t ht_nodes_without_parent(const struct ht_node* ht_tree, const uint64_t freq_len)
{
    uint64_t counter = 0;
    for (uint64_t i = 0; i < freq_len; i++) {
        /* A node without weight does not count, bc its not needed */
        if (ht_tree[i].weight == 0) continue;

        if (ht_tree[i].parent_node == NULL) counter += 1;
    }

    return counter;
}

/* Pass in the root, pre-order traversal (GePeTo lo dice) */
void ht_tree_print(const struct ht_node* ht_node) {
    if (ht_node == NULL) return;

    printf("%p: ", ht_node);
    print_ht_node(ht_node);
    ht_tree_print(ht_node->left_node);
    ht_tree_print(ht_node->right_node);
}

/* Make some texts to make sure that it keeps working throught the changes */
int main(void)
{
    const char* filepath = "main";
    // const char* filepath = "/home/xiri/Videos/2025-09-20_11-28-25.mkv"; 9.6Gb En 1s

    const size_t sorted_freq_len = UINT8_MAX + 1;
    struct char_freq* sorted_freq = char_freq_buffer_alloc();
    huffman_tree_calc_freq_from_file(filepath, sorted_freq, sorted_freq_len);

    PRINT_DEBUG("Print frequencies truncated\n");
    char_freq_buffer_print_truncated(sorted_freq, sorted_freq_len);

    arena_t ht_arena = arena_create(8192 * 8);

    size_t ht_tree_size = sizeof(struct ht_node) * sorted_freq_len * 4;
    struct ht_node* ht_tree = arena_alloc(&ht_arena, ht_tree_size);

    PRINT_DEBUG("Creation of the leaf nodes of the tree\n");
    for (uint64_t i = 0; i < sorted_freq_len; i++) {
        struct ht_node node = {
            .c = sorted_freq[i].c,
            .weight = sorted_freq[i].freq,
            .parent_node = NULL,
            .left_node = NULL,
            .right_node = NULL,
            .is_leaf = true,
            .is_root = false,
        };
        ht_tree[i] = node;
    }

    PRINT_DEBUG("Creation of the nodes that are not leaf of the tree\n");
    uint64_t i = 0; /* idx in the flat tree */
    uint64_t j = sorted_freq_len;
    while (ht_nodes_without_parent(ht_tree, j) > 1) {

        if ( i == j - 1) {
            /* TODO: is this unreacheable? */
            PRINT_DEBUG(" La i ha llegado al final del arbol\n");
            break;
        }

        struct ht_node* less_frequent_node = &ht_tree[i];
        struct ht_node* second_less_frequent_node = &ht_tree[i + 1];

        if (less_frequent_node->weight == 0) {
            i += 1;
            continue;
        }
        if (second_less_frequent_node->weight == 0) {
            i += 2;
            continue;
        }

        struct ht_node new_node = {
            .c = (unsigned char)0,
            .weight = less_frequent_node->weight + second_less_frequent_node->weight,
            .parent_node = NULL,
            .left_node = less_frequent_node,
            .right_node = second_less_frequent_node,
            .is_leaf = false,
            .is_root = false,
        };
        /* TODO: Alloc with the arena to make sure it fits, maybe a dynarray its better */
        ht_tree[j] = new_node;
        less_frequent_node->parent_node = &ht_tree[j];
        second_less_frequent_node->parent_node = &ht_tree[j];
        i += 2;
        j += 1;
        printf("%lu\n", ht_nodes_without_parent(ht_tree, j));
    }

    printf("ht_nodes_without_parent = %lu\n", ht_nodes_without_parent(ht_tree, j));
    printf("last i = %lu, last j = %lu\n", i, j);

    /* El ultimo nodo es claramente el root_node */
    ht_tree[i].is_root = true;
    print_ht_node(&ht_tree[i]);

    printf("the tree is %lu bytes\n", sizeof(struct ht_node) * j);
    printf("the arena is %d bytes\n", 8192 * 8);

    ht_tree_print(&ht_tree[i]);

    arena_destroy(&ht_arena);
    
    char_freq_buffer_free(sorted_freq);

    return 0;
}
