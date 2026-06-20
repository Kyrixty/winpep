#ifndef X86_H
#define X86_H
#include "common.h"
#include "arena.h"
#include "list.h"
#include <string.h>

typedef enum {
    add,
    adc,
    push,
    pop,
    UNKNOWN_MMC,
} mnemonic;

/* May just want letters for register names */
// Byte Register
typedef enum {
    al,
    cl,
    dl,
    bl,
    ah,
    ch,
    dh,
    bh,
} r8;

// Word Register
typedef enum {
    ax,
    cx,
    dx,
    bx,
    sp,
    bp,
    si,
    di,
} r16;

typedef enum {
    eax,
    ecx,
    edx,
    ebx,
    esp,
    ebp,
    esi,
    edi,
} r32;

// Quadword Register
typedef enum {
    rax,
    rcx,
    rdx,
    rbx,
    rsp,
    rbp,
    rsi,
    rdi,
} r64;
typedef enum {
    REG,
    MEM,
    IMM,
} opType;
typedef union {
    r8 byteReg;
    r16 wordReg;
    r32 dWordReg;
    r64 qWordReg;
    u8 imm8;
    u16 imm16;
    u32 imm32;
    u64 imm64;
} opVal;
 
typedef struct {
    opType type;
    opVal val;
    u32 valSize;
} operand;


/*
A REX prefix must be encoded when:

using 64-bit operand size and the instruction does not default to 64-bit operand size; or
using one of the extended registers (R8 to R15, XMM8 to XMM15, YMM8 to YMM15, CR8 to CR15 and DR8 to DR15); or
using one of the uniform byte registers SPL, BPL, SIL or DIL.

A REX prefix must not be encoded when:

using one of the high byte registers AH, CH, BH or DH.
In all other cases, the REX prefix is ignored. The use of multiple REX prefixes is undefined, although processors seem to use only the last REX prefix.

Instructions that default to 64-bit operand size in long mode are:

CALL (near)	    ENTER	    Jcc
JrCXZ	        JMP (near)	LEAVE
LGDT	        LIDT	    LLDT
LOOP	        LOOPcc	    LTR
MOV CR(n)	    MOV DR(n)	POP reg/mem
POP reg	        POP FS	    POP GS
POPFQ	        PUSH imm8   PUSH imm32
PUSH reg/mem	PUSH reg	PUSH FS
PUSH GS	        PUSHFQ	    RET (near)
*/
typedef struct {
    u8 pattern;
    u8 W;
    u8 R;
    u8 X;
    u8 B;
} REX;

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

typedef struct {
    u32 offset;
    u32 opcode;
    REX rex;
    ModRM mrm;
    SIB sib;
    mnemonic mmc;
    operand op1;
    operand op2;
    operand op3;
    operand op4;
    u32 nOps;
    u32 size_bytes;
    b32 hasRex;
} x86Instr_t;


void print_x86(x86Instr_t instr);
x86Instr_t* disassemble(arena_t* arena, u8* blob, u32 blobSize, u32* outNInstrs);

#endif