#include <file.h>
#include <ASSERT.h>

/* TODO: Maybe return erros */
void file_open(struct file_t* file, const char* restrict filepath)
{
    FILE* f = fopen(filepath, "rb");
    ASSERT(f != NULL /* Error opening file */);

    int ret_seek_end = fseek(f, 0, SEEK_END);
    ASSERT(ret_seek_end != -1 /* Error seeking to end of file, look at errno*/);

    long size_of_file = ftell(f);
    ASSERT(size_of_file != -1 /* Error getting size of file, look at errno*/);

    int ret_seek_begging = fseek(f, 0, SEEK_SET);
    ASSERT(ret_seek_begging == 0 /* Error seeking to beggining of file, look at errno*/);

    file->file = f;
    file->size = (uint64_t)size_of_file;
    file->seek_ptr = 0;
}

void file_read_to_uint8_t_buffer(struct file_t* file, uint8_t* restrict buffer, const uint32_t chars_to_read)
{
    ASSERT(file != NULL);
    ASSERT(buffer != NULL);
    ASSERT(file->size >= chars_to_read);
    ASSERT((file->size - file->seek_ptr) >= chars_to_read);

    unsigned long items_read = fread(buffer, sizeof(unsigned char), chars_to_read, file->file);
    ASSERT(items_read == chars_to_read /* Creo que look at errno XD */);

    file->seek_ptr += items_read;
}

void file_close(const struct file_t* file)
{
    ASSERT(file != NULL);
    fclose(file->file);
}
