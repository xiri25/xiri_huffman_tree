#ifndef M_ARENA_H
#define M_ARENA_H

#include <stddef.h>

typedef struct arena {
    void* data;
    size_t capacity;
    size_t offset;
} arena_t;

arena_t arena_create(size_t capacity);
void* arena_alloc(arena_t* arena, size_t size);
void arena_free(arena_t* arena);
void arena_destroy(arena_t* arena);

#endif // !M_ARENA_H
