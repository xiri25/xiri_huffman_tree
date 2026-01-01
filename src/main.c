#include "huffman_encoding.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <frequencies.h>
#include <huffman_tree.h>
#include <memory/arena.h>

#include <ASSERT.h>

/*
 * TODO: Deberia crear un diccionario con el huffman_code para cada caracter
 * y usar ese diccionario para en el encoding y decoding, ahorrandome atravesar
 * el arbol.
 * Puede que tambien quiera una implementacion atravesando el arbol para
 * usar menos memoria RAM
 * A la hora de crear el diccionario usar Depth‑first leaf‑only traversal
 * (es recursivo, pero hay manera de hacerlo iterativo)
*/

#define PRINT_DEBUG(x) printf(x);
// #define PRINT_DEBUG(x)

static int64_t root_weight_minus_freq(const struct ht_tree* tree,
                                       const struct char_freq* char_freq,
                                       const size_t char_freq_count)
{
    int64_t root_minus_chars_freqs = (int64_t)tree->tree[tree->root_idx].weight;
    for (size_t i = 0; i < char_freq_count; i++) {
        root_minus_chars_freqs -= (int64_t)char_freq[i].freq;
    }

    return root_minus_chars_freqs;
}

// /* Numero de hilos */
// #include <unistd.h>
// static long cpu_cores_online(void)
// {
//     long n = sysconf(_SC_NPROCESSORS_ONLN);
//     if (n < 1) n = 1;
//     return n;
// }

/* Make some texts to make sure that it keeps working throught the changes */
int main(int32_t argc, char** argv)
{
    (void)argc;
    const char* filepath = argv[1];

    const size_t sorted_freq_len = UINT8_MAX + 1;
    struct char_freq* sorted_freq = char_freq_buffer_alloc();
    huffman_tree_calc_freq_from_file(filepath, sorted_freq, (uint16_t)sorted_freq_len);

    char_freq_buffer_print_truncated(sorted_freq, sorted_freq_len);

    arena_t ht_arena = arena_create(8192 * 8);

    /*
     * NOTE: The tree can be as large as 2*(leaf_nodes) - 1
     * x + x/2 + x/4 + x/8 ..... + 1
    */
    const size_t ht_tree_size = sizeof(struct ht_node) * (sorted_freq_len * 2 - 1);
    struct ht_node* ht_tree_buffer = arena_alloc(&ht_arena, ht_tree_size);

    struct ht_tree tree = huffman_tree_create(sorted_freq, sorted_freq_len, ht_tree_buffer);

    ht_tree_print(&tree, false);

    ASSERT(root_weight_minus_freq(&tree, sorted_freq, sorted_freq_len) == 0);

    struct ht_dict* dict = arena_alloc(&ht_arena, sizeof(struct ht_dict) * 256);

    // FIXME: without memset it fails in some files, idk why just in some files
    memset(dict, 0, 256 * sizeof(struct ht_dict));
    huffman_dict_create(&tree, dict);

    ht_dict_print_truncated(dict, 256);
    
    /*
     * NOTE: To compare
     * https://suhaan-bhandary.github.io/Huffman-Coding/
     * No lo ordena como yo, por lo que los codigos generados
     * sin distintos, pero equivalentes (creo)
     */
    /*
     * NOTE: This structs are not packet, so maybe a ittle smaller
     * after serialization
     */
    const uint64_t original_size_bits = tree.tree[tree.root_idx].weight * sizeof(uint8_t) * 8;
    const uint8_t tree_header_size_bits = (sizeof(struct ht_tree) - sizeof(struct ht_node*)) * 8;
    const uint64_t tree_size_bits = tree.node_count * sizeof(struct ht_node) * 8;
    uint64_t compress_size_wo_tree_bits = 0;

    for (uint16_t i = 0; i < 256; i++) {
        uint64_t repeticions = 0;
        // XD
        for (uint16_t j = 0; j < sorted_freq_len; j++) {
            if (sorted_freq[j].c == i) {
                repeticions = sorted_freq[j].freq;
            }
        }
        compress_size_wo_tree_bits += dict[i].len * repeticions;
    }

    /*
     * TODO: Check size of original file
     * FIXME: El video grande produce todos los codigos con len 8 ??? idk what should happen
     * FIXME: Cat file | wc -c no siempre coincide con original_size_bits / 8
     */

    printf("original_size_bits =  %lu, compress_size_wo_tree_bits = %lu, compress_size_w_tree_bits = %lu\n",
           original_size_bits, compress_size_wo_tree_bits,
           compress_size_wo_tree_bits + tree_size_bits + tree_header_size_bits);

    printf("arena: offset(used) = %lu Bytes, capacity = %lu Bytes, used(%%) = %lf %%\n",
           ht_arena.offset, ht_arena.capacity,
           ((double)ht_arena.offset / (double)ht_arena.capacity) * 100.0);

    arena_destroy(&ht_arena);

    char_freq_buffer_free(sorted_freq);

    return 0;
}
