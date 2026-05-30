#include "x86.h"

typedef enum {
    eax,
    ecx,
    edx,
    ebx,
    sib,
    disp32,
    esi,
    edi,
} ModRM;

/**
 * 0x48 = REX.W (Register-EXtension (32-bit -> 64-bit))
 */
static char* OPCODE_MAP[] = {
    [0x00] = "ADD"
};

x86Instr_t disassemble(arena_t* arena, u8* blob) {
    /**
     * An instruction is of the format:
     * [prefixes] [REX] [Opcode] [ModR/M] [SiB] [Displacement] [Immediate]
     *     ??     0,1,2   1       1        0,1    0,1,2,4        0,1
     */
}