#include "x86.h"

/**
 * 0x48 = REX.W (Register-EXtension (32-bit -> 64-bit))
 */


static const u8 OPCODE_HAS_MRM[] = {
    [0x90] = false,
};

typedef struct {
    u8 mod;
    u8 regOrOpcode;
    u8 regOrMem;
} ModRM;

typedef struct {
    u8 scale;
    u8 index;
    u8 base;
} SIB;

static ModRM parse_mod_rm(u8 byte) {
    return (ModRM) {
        .mod            = byte & 0b11000000,
        .regOrOpcode    = byte & 0b00111000,
        .regOrMem       = byte & 0b00000111
    };
}

static SIB parse_sib(u8 byte) {
    return (SIB) {
        .scale  = byte & 0b11000000,
        .index  = byte & 0b00111000,
        .base   = byte & 0b00000111
    };
}

b32 has_mod_rm(u8 opcode) {
    return OPCODE_HAS_MRM[opcode];
}

u8 get_opcode_size(u8 b1, u8 b2) {
    u8 ret = 0;
    if (b1 != 0x0f) ret = 1;
    else if (b1 == 0x0f && b2 != 0x38 && b2 != 0x3a) ret = 2;
    else ret = 3;
    return ret;
}

/** 
 * To index an operand, pass its value
 * plus it's size (valSize)
*/
static const char* REG_NAME_MAP[] = {
    /* byte regs */
    [al + 8] = "al",
    [cl + 8] = "cl",
    [dl + 8] = "dl",
    [bl + 8] = "bl",
    [ah + 8] = "ah",
    [ch + 8] = "ch",
    [dh + 8] = "dh",
    [bh + 8] = "bh",

    /* word regs */
    [ax + 16] = "ax",
    [cx + 16] = "cx",
    [dx + 16] = "dx",
    [bx + 16] = "bx",
    [sp + 16] = "sp",
    [bp + 16] = "bp",
    [si + 16] = "si",
    [di + 16] = "di",

    /* dword regs */
    [eax + 32] = "eax",
    [ecx + 32] = "ecx",
    [edx + 32] = "edx",
    [ebx + 32] = "ebx",
    [esp + 32] = "esp",
    [ebp + 32] = "ebp",
    [esi + 32] = "esi",
    [edi + 32] = "edi",

    /* qword regs */
    [rax + 64] = "rax",
    [rcx + 64] = "rcx",
    [rdx + 64] = "rdx",
    [rbx + 64] = "rbx",
    [rsp + 64] = "rsp",
    [rbp + 64] = "rbp",
    [rsi + 64] = "rsi",
    [rdi + 64] = "rdi",
};

static const char* MMC_NAME_MAP[] = {
    [add] = "add",
    [push] = "push",
    [pop] = "pop",
    [UNKNOWN_MMC] = "???",
};

static const char* regToStr(opVal reg, u32 regSize) { return REG_NAME_MAP[reg.qWordReg + regSize]; }
static const char* mmcToStr(mnemonic mmc) { return MMC_NAME_MAP[mmc]; };

void print_x86(x86Instr_t instr) {
    printf("%s %s\n",
        mmcToStr(instr.mmc),
        regToStr(instr.op1.val, instr.op1.valSize));
}

typedef struct {
    bool ok;
    void* val;
    char* detail;
} Result;

#define Try(result) ((result).ok)

static inline u8 peekAt(u8* blob, u32 offset) { return blob[offset]; }
static inline u8 peekNext(u8* blob) { return peekAt(blob, 0); }

static mnemonic getMnemonicByOpcode(u32 opcode) {
    /* If we need to speedup can use a table for MSB and a table for LSB */
    if (WITHIN_RANGE(opcode, 0x50, 0x57)) return push;
    return UNKNOWN_MMC;
}

static Result _realDisasm(u8* blob, u32 blobSize, x86Instr_t* out) {
    Result r = {0};
    u8 b1 = peekAt(blob, 0), b2 = peekAt(blob, 1), b3 = peekAt(blob, 2); // BUG: ignoring < 3 opcodes at the end

    if (blobSize < 3) { r.detail = "mmm yumy blob consumed\n"; goto ReturnResult; }
    u8 opSize = get_opcode_size(b1, b2);
    u32 opcode = 0;
    for (u8 i = 0; i < opSize; i++) {
        opcode |= (u32)blob[i] << 8 * i;
    }

    // decode
    mnemonic mmc = getMnemonicByOpcode(opcode);
    u32 valSize = 0;
    u32 nParsed = 0;
    switch (mmc) {
        case push:
        {
            valSize = 32;
            // TODO: handle operands
            out->op1 = (operand) {.type = REG, .val = opcode - 0x50, .valSize = valSize};
            nParsed = 0;
        }; break;
        case UNKNOWN_MMC: { r.detail = "Unknown opcode"; } // fallthru
        default: { goto ReturnResult; }; break;
    }

    out->opcode = opcode;
    out->mmc = mmc;
    out->size_bytes = opSize + nParsed;
    r.ok = true;
    r.val = out;


ReturnResult:
    return r;
}

x86Instr_t* disassemble(arena_t* arena, u8* blob, u32 blobSize, u32* outNInstrs) {
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

    Result r = {0};
    *outNInstrs = 0;
    x86Instr_t* ret = ALLOC_STRUCT(arena, x86Instr_t);
    x86Instr_t* v = ret;
    while (Try((r = _realDisasm(blob, blobSize, v)))) {
        blob += ((x86Instr_t*)r.val)->size_bytes;
        v = ALLOC_STRUCT(arena, x86Instr_t);
        *outNInstrs = *outNInstrs + 1;
    }
    return ret;
}