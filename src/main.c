#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <file.h>
#include <frequencies.h>

void print_bits_form_uint8_t(const uint8_t byte)
{
    for (uint32_t i = 0; i < 8; i++) {
        printf("%d", (byte >> (7 - i)) & 1);
    }
}

int main(void)
{
    const char* filename = "hola.txt";
    struct file_t file = {};
    file_open(&file, filename);

    const uint32_t chars_to_read = (uint32_t)file.size;
    uint8_t* buffer = malloc(sizeof(unsigned char) * chars_to_read);

    file_read_to_uint8_t_buffer(&file, buffer, chars_to_read);

    for (uint32_t i = 0; i < chars_to_read; i++) {
        // NOTE: Can print a new line
        printf("%c: ", buffer[i]); // Esto provoca un cast (movzx)

        print_bits_form_uint8_t(buffer[i]);
        printf("\n");
    }

    struct char_freq* frequencies = char_freq_buffer_alloc();
    char_freq_buffer_create(frequencies, buffer, chars_to_read);
    struct char_freq* sorted_freq = char_freq_buffer_sorted(frequencies);
    char_freq_buffer_print(sorted_freq, chars_to_read);

    char_freq_buffer_free(frequencies);
    char_freq_buffer_free(sorted_freq);

    free(buffer);
    file_close(&file);
    return 0;
}
