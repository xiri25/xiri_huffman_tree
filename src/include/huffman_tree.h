#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <frequencies.h>

void huffman_tree_calc_freq_from_file(const char* filepath, struct char_freq* sorted_freq_buffer, const uint16_t buffer_size);

#endif // !HUFFMAN_TREE_H
