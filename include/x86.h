#ifndef X86_H
#define X86_H
#include "common.h"
#include "arena.h"
#include "list.h"
#include <string.h>

typedef enum {
    add,
    push,
    pop,
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
} opVal;
 
typedef struct {
    opType type;
    opVal val;
    u32 valSize;
} operand;

typedef struct {
    u32 offset;
    u32 opcode;
    mnemonic mmc;
    operand op1;
    operand op2;
    operand op3;
    operand op4;
    u32 nOps;
} x86Instr_t;


void print_x86(x86Instr_t instr);
x86Instr_t* disassemble(arena_t* arena, u8* blob, u32 blobSize, u32* outNInstrs);

#endif