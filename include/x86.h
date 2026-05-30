#ifndef X86_H
#define X86_H
#include "common.h"
#include "arena.h"

typedef struct {
    u32 offset;
    char* str;
} x86Instr_t;

x86Instr_t disassemble(arena_t* arena, u8* blob);

#endif