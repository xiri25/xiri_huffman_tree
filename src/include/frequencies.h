#ifndef FREQUENCIES_H
#define FREQUENCIES_H

#include <stdint.h>

struct char_freq {
    uint64_t freq;
    unsigned char c;
};

enum sort_order {
    SORT_ASC,
    SORT_DESC,
};

struct char_freq* char_freq_buffer_alloc();
void char_freq_buffer_free(const struct char_freq* buffer);
void char_freq_buffer_sort(struct char_freq* old_buffer,
                           struct char_freq* new_buffer,
                           const uint16_t buffer_size,
                           const enum sort_order sort_order);
void char_freq_buffer_calculate(struct char_freq* freq_buffer, const uint8_t* in_buffer, const uint64_t in_buffer_size);
void char_freq_buffer_print(const struct char_freq* buffer, const uint64_t buffer_size);
void char_freq_buffer_print_truncated(const struct char_freq* buffer, const uint64_t buffer_size);

#endif // !FREQUENCIES_H
