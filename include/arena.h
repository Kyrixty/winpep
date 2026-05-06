#ifndef RCC_ARENA_H
#define RCC_ARENA_H
#include "common.h"

typedef struct arena
{
    u64 capacity;
    u64 pos;
} arena_t;

#define ARENA_ERR_OUTOFMEM 1

// Credit: magicalbat

#define ALLOC_STRUCT(arena, T) ((T*)arena_alloc((arena), 1, sizeof(T), false))
#define ALLOC_STRUCT_NZ(arena, T) ((T*)arena_alloc((arena), 1, sizeof(T), true))
#define ALLOC_ARRAY(arena, T, n) ((T*)arena_alloc((arena), (n), sizeof(T), false))
#define ALLOC_ARRAY_NZ(arena, T) ((T*)arena_alloc((arena), (n), sizeof(T), true))

/**
 * Initializes arena with capacity `capacity`.
 */
arena_t*    arena_init(u64 capacity);
/**
 * Destroys the arena. Any data stored in the data
 * should be presumed to be junk after this function returns.
 */
void       arena_destroy(arena_t* arena);
/**
 * Pushes memory onto the arena stack.
 * Returns NULL if fail, otherwise a pointer to the memory 
 * allocated in the arena.
 * 
 * If `non_zero` is true, the returned data should be presumed to be junk.
 * If `non_zero` is false, the returned data will be all zeroes.
 */
void*       arena_alloc(arena_t* arena, u64 n_elems, u64 elem_size, b32 non_zero);
void        arena_pop(arena_t* arena, u64 size);
void        arena_pop_to(arena_t* arena, u64 pos);
void        arena_clear(arena_t* arena);
#endif
