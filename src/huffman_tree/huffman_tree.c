#include <huffman_tree.h>
#include <file.h>
#include <frequencies.h>

#include <stdlib.h>

struct char_freq* huffman_tree_calc_freq_from_file(const char* filepath)
{
    struct file_t file = {};
    file_open(&file, filepath);

    const uint32_t chars_to_read = (uint32_t)file.size;
    uint8_t* file_buffer = malloc(sizeof(unsigned char) * chars_to_read);

    file_read_to_uint8_t_buffer(&file, file_buffer, chars_to_read);

    struct char_freq* frequencies = char_freq_buffer_alloc();
    char_freq_buffer_create(frequencies, file_buffer, chars_to_read);

    struct char_freq* sorted_freq = char_freq_buffer_sorted(frequencies);

    char_freq_buffer_free(frequencies);

    free(file_buffer);
    file_close(&file);

    return sorted_freq;
}
