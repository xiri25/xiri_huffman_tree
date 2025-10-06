#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stdio.h>

struct file_t {
    FILE* file;
    uint64_t size;
    uint64_t seek_ptr;
};

void file_open(struct file_t* file, const char* restrict filepath);
void file_read_to_uint8_t_buffer(struct file_t* file, uint8_t* restrict buffer, const uint32_t chars_to_read);
void file_close(const struct file_t* file);

#endif // !FILE_H
