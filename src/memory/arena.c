#include <memory/arena.h>
#include <ASSERT.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ALIGNMENT_VALUE 8

static bool is_power_of_2(size_t n)
{
    return n && !(n & (n - 1));
}

static size_t ptr_alignment_offset(const void* ptr, const size_t alignment)
{
    ASSERT(ptr != NULL);
    ASSERT(alignment != 0);
    ASSERT(is_power_of_2(alignment));

    uintptr_t u_ptr = (uintptr_t)ptr;
    size_t offset_to_align = 0;
    size_t current_offset = u_ptr % alignment;

    if (current_offset) {
        offset_to_align = alignment - current_offset;
    }

    return offset_to_align;
}

arena_t arena_create(size_t capacity)
{
    void* data = malloc(capacity);
    ASSERT(data != NULL /* malloc failed */);

    size_t need_to_align_offset = ptr_alignment_offset(data, ALIGNMENT_VALUE);

    arena_t a = {
        .data = data + need_to_align_offset,
        .capacity = capacity,
        .offset = need_to_align_offset
    };

    return a;
}

/* TODO: IDK if i want the memory always align, quite overhead in arena_alloc() */
void* arena_alloc(arena_t* arena, size_t size)
{
    ASSERT(arena != NULL);
    size_t need_to_align_offset = ptr_alignment_offset(arena->data + arena->offset, ALIGNMENT_VALUE);
    size_t total_size = size + need_to_align_offset;

    /*
     * TODO: IDK if assert or if check when assert deactivated,
     * parece que con -O2 solo compila el ASSERT(), quizas porque
     * es el primero en aparecer?
     */
    ASSERT((total_size + arena->offset) <= arena->capacity);
    if ((total_size + arena->offset) > arena->capacity) return NULL;

    void* block = (void*)((char*)arena->data + arena->offset + need_to_align_offset);
    arena->offset += total_size;

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
