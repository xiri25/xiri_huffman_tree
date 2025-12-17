#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

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

void print_bits_form_uint8_t(const uint8_t byte)
{
    for (uint32_t i = 0; i < 8; i++) {
        printf("%d", (byte >> (7 - i)) & 1);
    }
}

/* Make some texts to make sure that it keeps working throught the changes */
int main(int32_t argc, char** argv)
{
    (void)argc;
    const char* filepath = argv[1];

    const size_t sorted_freq_len = UINT8_MAX + 1;
    struct char_freq* sorted_freq = char_freq_buffer_alloc();
    huffman_tree_calc_freq_from_file(filepath, sorted_freq, sorted_freq_len);

    char_freq_buffer_print_truncated(sorted_freq, sorted_freq_len);

    arena_t ht_arena = arena_create(8192 * 8);

    /*
     * NOTE: The tree can be as large as 2*(leaf_nodes) - 1
     * x + x/2 + x/4 + x/8 ..... + 1
    */
    const size_t ht_tree_size = sizeof(struct ht_node) * (sorted_freq_len * 2 - 1);
    struct ht_node* ht_tree = arena_alloc(&ht_arena, ht_tree_size);

    struct ht_tree tree = huffman_tree_create(sorted_freq, sorted_freq_len, ht_tree);

    ht_tree_print(tree.root);

    arena_destroy(&ht_arena);

    char_freq_buffer_free(sorted_freq);

    return 0;
}
