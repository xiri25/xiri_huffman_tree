#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <memory/arena.h>
#include <ASSERT.h>

/*
 * TODO: Create a function that initializes
 * arena.data to a known pattern so i can
 * make sure that im using the memory that
 * im requesting with arena_alloc()
 * It is pretty difficult to do it bc
 * there is no way of keep track of
 * the suballocations and there is gonna be
 * some chuncks between suballocations
 * bc of memory alignment
 *
 * TODO: Este video:
 * https://www.youtube.com/watch?v=jgiMagdjA1s
*/

#define ALIGNMENT_VALUE 8

static bool is_power_of_2(size_t n)
{
    return n && !(n & (n - 1));
}

static size_t ptr_alignment_offset(const uintptr_t ptr, const size_t alignment)
{
    ASSERT(ptr != 0);
    ASSERT(alignment != 0);
    ASSERT(is_power_of_2(alignment));

    size_t offset_to_align = 0;
    size_t current_offset = ptr % alignment;

    if (current_offset) {
        offset_to_align = alignment - current_offset;
    }

    return offset_to_align;
}

/* https://www.youtube.com/watch?v=nij0hktapWM */
static size_t align_up(size_t value, size_t alignment)
{
    return ((value + alignment - 1) / alignment) * alignment;
}

/*
 * TODO: Its needs testing
 * A refactor to use align_up directly is better
 * Parece que estoy alineando siempre con potencias de 2
 * por lo que hay una implementacion aun mejor de align_up()
 * Casi que prefiero refactorizar arena_create() y arena_alloc()
 * me parece mas clean
*/
static size_t ptr_alignment_offset_v2(const void* ptr, const size_t alignment)
{
    ASSERT(ptr != NULL);
    ASSERT(alignment != 0);
    ASSERT(is_power_of_2(alignment));

    uintptr_t u_ptr = (uintptr_t)ptr;
    uintptr_t align_up_value = (uintptr_t)align_up(u_ptr, alignment);

    size_t offset_to_align = align_up_value - u_ptr;

    return offset_to_align;
}

arena_t arena_create(size_t capacity)
{
    void* data = malloc(capacity);
    ASSERT(data != NULL);

    size_t need_to_align_offset = 0;
    need_to_align_offset = ptr_alignment_offset((uintptr_t)data, ALIGNMENT_VALUE);

    arena_t a = {
        .data = (void*)((uintptr_t)data + need_to_align_offset),
        .capacity = capacity,
        .offset = need_to_align_offset
    };

    return a;
}

/* TODO: IDK if i want the memory always align, quite overhead in arena_alloc() */
void* arena_alloc(arena_t* arena, size_t size)
{
    ASSERT(arena != NULL);
    ASSERT(arena->data != NULL);

    const uintptr_t arena_ptr = (uintptr_t)arena->data + (uintptr_t)arena->offset;
    size_t need_to_align_offset = ptr_alignment_offset(arena_ptr, ALIGNMENT_VALUE);
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
    ASSERT(arena->data != NULL);

    free(arena->data);
}
