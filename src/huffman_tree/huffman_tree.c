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

/* No se me podia ocurrir una idea peor (creo) */
/*
 * Creo que puedo contabilizar cuantos nodos hay sin parent en cada paso
 * en plan al principio tengo x nodos sin parent, cojo 2 y creo un nuevo
 * nodo, ahora tengo dos nodos con parent y uno nuevo sin parent
*/
uint64_t ht_nodes_without_parent(const struct ht_node* ht_tree, const uint64_t freq_len)
{
    uint64_t counter = 0;
    for (uint64_t i = 0; i < freq_len; i++) {
        /* A node without weight does not count, bc its not needed */
        if (ht_tree[i].weight == 0) continue;

        if (ht_tree[i].parent_node == -1) counter += 1;
    }

    return counter;
}

struct ht_tree huffman_tree_create(const struct char_freq* sorted_freq,
                                   const size_t sorted_freq_len,
                                   struct ht_node* tree_buffer)
{
    /* TODO: Tener en cuenta que no hacen falta los nodos cuyo weight es 0 */
    for (uint64_t i = 0; i < sorted_freq_len; i++) {
        struct ht_node node = {
            .c           = sorted_freq[i].c,
            .weight      = sorted_freq[i].freq,
            .parent_node = -1,
            .left_node   = -1,
            .right_node  = -1,
        };
        tree_buffer[i] = node;
    }

    /*
     * FIXME: Estoy dejando muchos huecos sin usar al principio del array
     * el tree es oequeño, por ejemplo con hola.txt no tiene sentido
     * llegar hasta ~260 nodos
     */

    uint64_t i = 0; /* idx of the leaf nodes, they are at the beggining of the array */
    uint64_t j = sorted_freq_len; /* idx of the rest of the nodes */
    while (ht_nodes_without_parent(tree_buffer, j) > 1) {

        if ( i == j - 1) {
            /* TODO: is this unreacheable? */
            // PRINT_DEBUG(" La i ha llegado al final del arbol\n");
            ASSERT(0 /* unreacheable? */);
            break;
        }

        struct ht_node* less_frequent_node = &tree_buffer[i];
        struct ht_node* second_less_frequent_node = &tree_buffer[i + 1];

        if (less_frequent_node->weight == 0) {
            i += 1;
            continue;
        }
        if (second_less_frequent_node->weight == 0) {
            i += 2;
            continue;
        }

        struct ht_node new_node = {
            .c           = (unsigned char)0,
            .weight      = less_frequent_node->weight + second_less_frequent_node->weight,
            .parent_node = -1,
            .left_node   = (int16_t)i,
            .right_node  = (int16_t)i + 1,
        };
        /* TODO: Alloc with the arena to make sure it fits, maybe a dynarray its better */
        tree_buffer[j] = new_node;
        less_frequent_node->parent_node = (int16_t)j;
        second_less_frequent_node->parent_node = (int16_t)j;
        i += 2;
        j += 1;
    }
    /*
     * NOTE: The tree can be as large as 2*(leaf_nodes) - 1
     * x + x/2 + x/4 + x/8 ..... + 1
    */
    /* The index of the no leaf nodes should not be greater that the size of the tree */
    ASSERT(j <= sorted_freq_len * 2 - 1);

    const struct ht_tree tree = {
        .tree = tree_buffer,
        .root_idx = (uint16_t)i, /* El ultimo nodo es claramente el root_node */
        .node_count = (uint16_t)i + 1, /* TODO: los indices no necesitan ser uint64_t */
    };

    printf("ht_nodes_without_parent = %lu\n", ht_nodes_without_parent(tree_buffer, j));
    printf("last i = %lu, last j = %lu\n", i, j);
    printf("the tree is %lu bytes\n", sizeof(struct ht_node) * j);

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
