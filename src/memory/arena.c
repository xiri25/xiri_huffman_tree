#include <memory/arena.h>
#include <ASSERT.h>

#include <stdlib.h>

/* TODO: Maybe make some alignment considerations */

arena_t arena_create(size_t capacity)
{
    void* data = malloc(capacity);
    ASSERT(data != NULL /* malloc failed */);

    arena_t a = {
        .data = data,
        .capacity = capacity,
        .offset = 0
    };

    return a;
}

void* arena_alloc(arena_t* arena, size_t size)
{
    ASSERT(arena != NULL);

    /*
     * TODO: IDK if assert or if check when assert deactivated,
     * parece que con -O2 solo compila el ASSERT(), quizas porque
     * es el primero en aparecer?
     */
    ASSERT((size + arena->offset) <= arena->capacity);
    if ((size + arena->offset) > arena->capacity) return NULL;

    void* block = (void*)((char*)arena->data + arena->offset);
    arena->offset += size;

    return block;
}

void arena_free(arena_t* arena)
{
    ASSERT(arena != NULL);
    arena->offset = 0;
}

void arena_destroy(arena_t* arena)
{
    ASSERT(arena != NULL);
    free(arena->data);
}
