#ifndef FREQUENCIES_H
#define FREQUENCIES_H

#include <stdint.h>

struct char_freq {
    uint64_t freq;
    unsigned char c;
};

struct char_freq* char_freq_buffer_alloc();
void char_freq_buffer_free(const struct char_freq* buffer);
struct char_freq* char_freq_buffer_sorted(struct char_freq* buffer);
void char_freq_buffer_create(struct char_freq* freq_buffer, const uint8_t* in_buffer, const uint64_t in_buffer_size);
void char_freq_buffer_print(const struct char_freq* buffer, const uint64_t buffer_size);

#endif // !FREQUENCIES_H
