#include "arena.h"
#include <stdlib.h>
#include <memory.h>

#define ARENA_BASE_POS (sizeof(arena_t))
#define ARENA_ALIGN (WORD_SIZE)
#define ALIGN_UP_POW2(n, p) ((((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1))))

void arena_oom() {
    exit(ARENA_ERR_OUTOFMEM);
}

/**
 * Initializes arena with capacity `total_size`
 */
arena_t* arena_init(u64 capacity) {
    arena_t* arena = malloc(sizeof(arena_t) + capacity);
    arena->pos = ARENA_BASE_POS;
    arena->capacity = capacity;
    return arena;
}
/**
 * Destroys the arena. Any data stored in the data
 * should be presumed to be junk after this function returns.
 */
void arena_destroy(arena_t* arena) {
    free(arena);
}
void* arena_alloc(arena_t* arena, u64 n_elems, u64 elem_size, b32 non_zero) {
    u64 pos_align = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    u64 size_req = n_elems * elem_size;
    u64 pos_new = pos_align + size_req;

    if (pos_new > arena->capacity) { arena_oom(); return NULL; }
    arena->pos = pos_new;
    u8* out = (u8*)arena + pos_align;

    if (!non_zero) {
        memset(out, 0, size_req);
    }
    return out;
}
void arena_pop(arena_t* arena, u64 size) {
    size = MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}
void arena_pop_to(arena_t* arena, u64 pos) {
    u64 size = pos < arena->pos ? arena->pos - pos : 0;
    arena_pop(arena, size);
}
void arena_clear(arena_t* arena) {
    arena_pop_to(arena, ARENA_BASE_POS);
}