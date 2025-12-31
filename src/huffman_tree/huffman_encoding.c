#include <huffman_encoding.h>
#include <ASSERT.h>

#include <stdint.h>
#include <stdio.h>

void bits_form_uint8_t_to_str(const uint8_t byte, char* str) {
    for (uint32_t i = 0; i < 8; i++) {
        str[i] = ((byte >> (7 - i)) & 1) ? '1' : '0';
    }
    str[8] = '\0';
}

void bits_form_uint16_t_to_str(const uint16_t bytes, char* str) {
    for (uint32_t i = 0; i < 16; i++) {
        str[i] = ((bytes >> (15 - i)) & 1) ? '1' : '0';
    }
    str[16] = '\0';
}

void build_huffman_dict_recursive(const struct ht_tree* tree,
                                  const struct ht_node* node,
                                  struct ht_dict* dict,
                                  struct ht_dict current_code)
{
    if (node == NULL) return;

    printf("node idx = %lu -> ", ((uintptr_t)node - (uintptr_t)tree->tree) / (sizeof(struct ht_node)));
    char current_code_code_str[17] = {};

    if (ht_node_is_leaf(node)) {
        ASSERT(current_code.len > 0);
        ASSERT(current_code.len < 17);
        dict[(uint8_t)node->c] = current_code;
        bits_form_uint16_t_to_str(current_code.code, current_code_code_str);
        printf("Leaf node, char = %d, code = %s, len= %d\n", node->c, current_code_code_str, current_code.len);
        return;
    }

    // Traverse left: Add a 0 to current_code
    current_code.code <<= 1;
    current_code.len++;
    bits_form_uint16_t_to_str(current_code.code, current_code_code_str);
    printf("left was taken, current_code = {.code = %s, .len = %d}\n", current_code_code_str, current_code.len);
    build_huffman_dict_recursive(tree, &tree->tree[node->left_node], dict, current_code);
    ASSERT(current_code.len > 0);
    ASSERT(current_code.len < 17);
    // Remove the last bit for backtracking -> shift right
    current_code.len--;
    current_code.code >>= 1;

    // Traverse right: Add a 1 to current_code
    current_code.code = (uint8_t)(current_code.code << 1) | 1;
    current_code.len++;
    bits_form_uint16_t_to_str(current_code.code, current_code_code_str);
    printf("right was taken, current_code = {.code = %s, .len = %d}\n", current_code_code_str, current_code.len);
    build_huffman_dict_recursive(tree, &tree->tree[node->right_node], dict, current_code);
    ASSERT(current_code.len > 0);
    ASSERT(current_code.len < 17);
    // Remove the last bit for backtracking -> shift right
    current_code.len--;
    current_code.code >>= 1;
}

void ht_dict_print_truncated(const struct ht_dict* dict, const uint16_t size)
{
    ASSERT(size == 256);
    for (uint16_t i = 0; i < size; ++i) {
        if (dict[i].len != 0) {
            char current_code_code_str[17] = {};
            bits_form_uint16_t_to_str(dict[i].code, current_code_code_str);
            ASSERT(dict[i].len < 17);
            printf("current_code = char = %d {.code = %s, .len = %d}\n", i, current_code_code_str, dict[i].len);
        }
    }
}
