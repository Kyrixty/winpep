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
     * [prefixes] [Opcode] [ModR/M] [SiB] [Displacement] [Immediate]
     * Where:
     * [prefixes]       = up to 4 prefixes, 1 byte each (REX is found here)
     * [Opcode]         = 1, 2, or 3-byte opcode
     * [Mod R/M]        = 1 byte if required
     * [SiB]            = 1 byte if required
     * [Displacement]   = address displacement (from memory) of 1, 2, or 4 bytes or none
     * [Immediate]      = Immediate data of 1, 2, or 4 bytes or none
     * 
     * ------- Layout
     * [Mod R/M] (if required) is 1 byte
     * where:
     * 7   6 5         3 2  0
     * [Mod][Reg/Opcode][R/M]
     * 
     * [Mod]        = 
     * [Reg/Opcode] =
     * [R/M]        = 
     * 
     * [SiB] (if required) is 1 byte
     * where:
     * 7    6  5    3 2   0
     * [Scale][Index][Base]
     * is calculated using x = 
     */
}