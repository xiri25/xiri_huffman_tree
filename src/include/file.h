#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stdio.h>

struct file_t {
    FILE* file;
    uint64_t size;
    uint64_t seek_ptr;
};

struct file_raw_t {
    int32_t fd;
    size_t size;
};

int32_t file_raw_open(struct file_raw_t* file, const char* filepath);
void file_raw_close(const struct file_raw_t* file);
struct file_t file_open(const char* filepath);
void file_read_to_uint8_t_buffer(struct file_t* file, uint8_t* restrict buffer, const size_t chars_to_read);
void file_close(const struct file_t* file);

#endif // !FILE_H
