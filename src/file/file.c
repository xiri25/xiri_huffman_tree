#include <file.h>
#include <ASSERT.h>

#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/* TODO: Maybe return erros */
/*
 * TODO: Rewrite this. Tener en cuenta que no creo que haga falta
 * tener el seek_ptr , porque struct FILE deberia tener esa
 * informacion
*/
int32_t file_raw_open(struct file_raw_t* file, const char* filepath)
{
    const int32_t fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }

    struct stat file_stat = {};
    if (fstat(fd, &file_stat) < 0) {
        perror("fstat error");
        return -1;
    }

    file->fd = fd;
    file->size = (size_t)file_stat.st_size;

    return 0;
}

void file_raw_close(const struct file_raw_t* file)
{
    ASSERT(file != NULL);
    close(file->fd);
}

struct file_t file_open(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");
    ASSERT(f != NULL /* Error opening file */);

    int ret_seek_end = fseek(f, 0, SEEK_END);
    ASSERT(ret_seek_end != -1 /* Error seeking to end of file, look at errno*/);

    long size_of_file = ftell(f);
    ASSERT(size_of_file != -1 /* Error getting size of file, look at errno*/);

    int ret_seek_begging = fseek(f, 0, SEEK_SET);
    ASSERT(ret_seek_begging == 0 /* Error seeking to beggining of file, look at errno*/);

    const struct file_t file = {
        .file = f,
        .size = (uint64_t)size_of_file,
        .seek_ptr = 0,
    };

    return file;
}

void file_read_to_uint8_t_buffer(struct file_t* file, uint8_t* restrict buffer, const size_t chars_to_read)
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
