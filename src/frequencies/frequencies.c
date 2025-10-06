#include <frequencies.h>
#include <ASSERT.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct char_freq* _char_freq_buffer_alloc(const uint16_t buffer_size)
{
    struct char_freq* buffer = malloc(sizeof(struct char_freq) * buffer_size);
    
    return buffer;
}

static void _char_freq_buffer_initialize(struct char_freq* buffer, const uint16_t buffer_size)
{
    for (uint16_t i = 0; i < buffer_size; i++) {
        buffer[i].freq = 0;
        buffer[i].c = (unsigned char)i;
    }
}

static uint16_t _char_freq_buffer_max_freq_idx(const struct char_freq* buffer, const uint16_t buffer_size)
{
    uint64_t max_freq = 0;
    uint16_t max_freq_idx = 0;

    for (uint16_t i = 0; i < buffer_size; i++) {
        if (buffer[i].freq > max_freq) {
            max_freq = buffer[i].freq;
            max_freq_idx = i;
        }
    }

    return max_freq_idx;
}

static void _char_freq_print(const struct char_freq freq)
{
    /*
    const char line_break = '\n';
    if (freq.c == line_break) {
        printf("'\\n': %lu\n", freq.freq);
    } else {
        printf("'%c': %lu\n", freq.c, freq.freq);
    }
    */
    printf("%d: %lu\n", freq.c, freq.freq);
}

struct char_freq* char_freq_buffer_alloc()
{
    const uint16_t buffer_size = UINT8_MAX + 1;
    struct char_freq* buffer = _char_freq_buffer_alloc(buffer_size);

    _char_freq_buffer_initialize(buffer, buffer_size);

    return buffer;
}

void char_freq_buffer_free(const struct char_freq* buffer)
{
    ASSERT(buffer != NULL);
    free((void*)buffer);
}

// TODO: This is very slow
struct char_freq* char_freq_buffer_sorted(struct char_freq* buffer)
{
    ASSERT(buffer != NULL);

    const uint16_t buffer_size = UINT8_MAX + 1;
    struct char_freq* sorted_buffer = _char_freq_buffer_alloc(buffer_size);

    for (uint16_t i = 0; i < buffer_size; i++) {
        uint16_t old_buffer_idx = _char_freq_buffer_max_freq_idx(buffer, buffer_size);
        sorted_buffer[i] = buffer[old_buffer_idx];

        // To not pick this one again
        buffer[old_buffer_idx].freq = 0;
    }

    return sorted_buffer;
}

// TODO: Better name
void char_freq_buffer_create(struct char_freq* freq_buffer, const uint8_t* char_buffer, const uint64_t char_buffer_size)
{
    ASSERT(freq_buffer != NULL);
    ASSERT(char_buffer != NULL);
    ASSERT(char_buffer_size > 0);

    for (uint64_t i = 0; i < char_buffer_size; i++) {
        uint8_t freq_buffer_idx = char_buffer[i];
        freq_buffer[freq_buffer_idx].freq += 1;
    }
}

void char_freq_buffer_print(const struct char_freq* buffer, const uint64_t buffer_size)
{
    for (uint64_t i = 0; i < buffer_size; i++) {
        _char_freq_print(buffer[i]);
    }
}

void char_freq_buffer_print_truncated(const struct char_freq* buffer, const uint64_t buffer_size)
{
    for (uint64_t i = 0; i < buffer_size; i++) {
        if (buffer[i].freq > 0) {
            _char_freq_print(buffer[i]);
        }
    }
}
