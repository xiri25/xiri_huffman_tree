#include <huffman_tree.h>
#include <file.h>
#include <frequencies.h>

#include <stdlib.h>
#include <string.h>

/* TODO: check if the char is printable (or \n \0...) and print it */
void ht_node_print(const struct ht_node* node)
{
    char is_root[5] = {};
    if (node->is_root) {
        strcpy(is_root, "root\0");
    }
    if (node->is_leaf) {
        strcpy(is_root, "leaf\0");
    }
    printf("%d: %lu, p: %p, l: %p, r: %p, %s\n",
           node->c, node->weight, node->parent_node,
           node->left_node, node->right_node, is_root);
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
