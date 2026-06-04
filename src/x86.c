#include "x86.h"

/**
 * 0x48 = REX.W (Register-EXtension (32-bit -> 64-bit))
 */

DECL_LIST(x86List, x86Instr_t);

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

static char* regToStr(opVal reg, u32 regSize) {
    return REG_NAME_MAP[reg.qWordReg + regSize];
}

void print_x86(x86Instr_t instr) {
    switch (instr.mmc) {
        case push: {
            char* rhs = "???";
            if (WITHIN_RANGE(instr.opcode, 0x50, 0x57)) {
                rhs = regToStr(instr.op1.val, instr.op1.valSize);
            }
            printf("push\t%s\n", rhs);
        }; break;
        default: { printf("???\n"); };
    }
}

void set_op_1(x86Instr_t* out, opType type, u32 val, u32 valSize) { out->op1 = (operand) {.type = type, .val = val, .valSize = valSize}; }
void set_op_2(x86Instr_t* out, opType type, u32 val, u32 valSize) { out->op2 = (operand) {.type = type, .val = val, .valSize = valSize}; }
void set_op_3(x86Instr_t* out, opType type, u32 val, u32 valSize) { out->op3 = (operand) {.type = type, .val = val, .valSize = valSize}; }
void set_op_4(x86Instr_t* out, opType type, u32 val, u32 valSize) { out->op4 = (operand) {.type = type, .val = val, .valSize = valSize}; }

/**
 * Parses a 1-byte opcode x86 instruction.
 * 
 * Pass blob after opcode.
 */
static i32 __parse_1byte_x86(u32 opcode, u8* blob, u32 blobSize, x86Instr_t* out) {
    i32 bytes_parsed = -1;
    u32 valSize = 8;
    // push reg
    if (WITHIN_RANGE(opcode, 0x50, 0x57)) {
        out->mmc = push;
        out->nOps = 1;
        valSize = 32;
        set_op_1(out, REG, opcode - 0x50, valSize);
        bytes_parsed = 0; // jank? 
    }
    return bytes_parsed;
}

/**
 * Parses a 2-byte opcode x86 instruction.
 * 
 * Pass blob after opcode.
 */
static i32 __parse_2byte_x86(u32 opcode, u8* blob, u32 blobSize, x86Instr_t* out) {
    return -1;
}

/**
 * Parses a 3-byte opcode x86 instruction.
 * 
 * Pass blob after opcode.
 */
static i32 __parse_3byte_x86(u32 opcode, u8* blob, u32 blobSize, x86Instr_t* out) {
    return -1;
}

/**
 * Parses 1 instruction in the blob, returning how many bytes were
 * parsed.
 * 
 * Returns -1 on error
 */
static i32 __disassemble(u8* blob, u32 blobSize, x86Instr_t* out) {
    u32 i = 0;
    u8 byte = blob[i];
    u8 b1 = byte, b2 = 0, b3 = 0;
    
    // ----------- OPCODE -----------
    // First get opcode size
    if (blobSize >= 3) {
        b2 = blob[i + 1];
        b3 = blob[i + 2];
    }
    else if (blobSize > 1) {
        b2 = blob[i + 1];
    }
    u32 opcode_size = get_opcode_size(b1, b2);
    u32 opcode = 0;
    for (u32 j = 0; j < opcode_size; j++) {
        opcode |= blob[j] << 8 * j;
    }
    out->opcode = opcode;

    i32 nParsed = 0;
    switch (opcode_size) {
        case 1: { nParsed = __parse_1byte_x86(opcode, blob + opcode_size, blobSize, out); }; break;
        case 2: { nParsed = __parse_2byte_x86(opcode, blob + opcode_size, blobSize, out); }; break;
        case 3: { nParsed = __parse_3byte_x86(opcode, blob + opcode_size, blobSize, out); }; break;
        default: { return -1; }; break;
    }
    if (nParsed < 0) {
        return nParsed;
    }
    return opcode_size + nParsed;
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

     /**
      * I want to rework this system before it gets to clustered.
      * I don't see it going in a good direction in it's current state.
      * Perhaps implement parsing for a few more complex instructions
      * and then rethink how I want this disassembler to look.
      */
    x86Instr_t in = {0};
    x86List* list = x86List_init();
    u32 i = 0;
    u32 nInstrs = 0;
    while (i < blobSize) {
        i32 nParsed = __disassemble(blob + i, blobSize - i, &in);
        if (nParsed < 0)
            break;
        // printf("0x%x\n", in.opcode);
        x86List_append(list, in);
        i += nParsed;
        nInstrs++;
    }
    if (outNInstrs) *outNInstrs = nInstrs;
    x86Instr_t* copy = NULL;
    if (list->len == 0) {
        goto freeListAndReturn;
    }
    copy = ALLOC_ARRAY(arena, x86Instr_t, list->len);
    memcpy(copy, list->data, sizeof(x86Instr_t) * list->len);
freeListAndReturn:
    x86List_free(list);
    return copy;
}