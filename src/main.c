#include <stdio.h>
#include <stdint.h>
#include <frequencies.h>
#include <huffman_tree.h>

void print_bits_form_uint8_t(const uint8_t byte)
{
    for (uint32_t i = 0; i < 8; i++) {
        printf("%d", (byte >> (7 - i)) & 1);
    }
}

int main(void)
{
    const char* filepath = "main";
    struct char_freq* sorted_freq = huffman_tree_calc_freq_from_file(filepath);
    char_freq_buffer_print_truncated(sorted_freq, UINT8_MAX + 1);

    return 0;
}
