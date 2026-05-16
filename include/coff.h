#ifndef COFF_H
#define COFF_H

#include "common.h"
#include "fs.h"
#include "arena.h"
#include <stdlib.h>
                                        /* if set */
#define F_RELFLG    1 << 0      /* There is no relocation information in this file. Usually clear for object files and set for executables */
#define F_EXEC      1 << 1      /* All unresolved symbols have been resolved; this file is executable */
#define F_LNNO      1 << 2      /* Line number information has been removed or omitted */
#define F_LSYMS     1 << 3      /* All local symbols have been removed or omitted */
#define F_AR32WR    1 << 8      /* This file is 32-bit little endian */

#define FHDR_MAGIC_AMD64 0x8664

/* section types */
#define F_SHDR_TEXT 0x0020
#define F_SHDR_DATA 0x0040
#define F_SHDR_BSS  0x0080

/* Symbol Table */
/* Symbol `scnum` Special Types */
#define N_SMBL_UNDEF        0       /* extern symbol */
#define N_SMBL_ABS         -1       /* An absolute symbol (symEntry_t.value is a constant, not an address)*/
#define N_SMBL_DEBUG       -2       /* Debugging symbol */
/* Symbol Types */
/**
 * Symbol types are composed of a base type and a derived type.
 * The base type identifies what "T" is.
 * The derived type identifies any "modifiers" on that base type.
 * 
 * For example, if symEntry_t.type is composed of:
 * base type: T_SMBL_CHAR
 * derived type: DT_SMBL_PTR
 * Then you should interpret the actual type of the symbol 
 * as "pointer to char" or "char *" 
 */
#define T_SMBL_NULL         0       /* No symbol */
#define T_SMBL_VOID         1       /* void function argument (void f(args..) ?) */
#define T_SMBL_CHAR         2
#define T_SMBL_SHORT        3
#define T_SMBL_INT          4
#define T_SMBL_LONG         5
#define T_SMBL_FLOAT        6
#define T_SMBL_DOUBLE       7
#define T_SMBL_STRUCT       8
#define T_SMBL_UNION        9
#define T_SMBL_ENUM         10
#define T_SMBL_MOE          11      /* Member of enumeration (like Color.RED) */
#define T_SMBL_UCHAR        12
#define T_SMBL_USHORT       13
#define T_SMBL_UINT         14
#define T_SMBL_ULONG        15
#define T_SMBL_LNGDBL       16
/* Derived Type */
#define DT_SMBL_NON          0       /* No derived type (if T_SMBL_INT is the base then the symbol is an int) */
#define DT_SMBL_PTR          1       /* ptr to T (see M_SMBL_PTR) */
#define DT_SMBL_FCN          2       /* func returning T (see M_SMBL_FCN) */
#define DT_SMBL_ARY          3       /* array of T (see M_SMBL_ARY) */
/* Symbol Type Macros */
/**
 * You should use these macros to determine the type of a symbol.
 * If you want to use your own, symEntry_t.type = base type + derived type << 8
 */
#define SYMBOL_T(sym)       ((sym) & 0x0f)
#define SYMBOL_DT(sym)      ((sym) >> 8)
#define SYMBOL_IS_NON(sym)  (SYMBOL_DT(sym) == DT_SMBL_NON)
#define SYMBOL_IS_PTR(sym)  (SYMBOL_DT(sym) == DT_SMBL_PTR)
#define SYMBOL_IS_FCN(sym)  (SYMBOL_DT(sym) == DT_SMBL_FCN)
#define SYMBOL_IS_ARY(sym)  (SYMBOL_DT(sym) == DT_SMBL_ARY)
/* Symbol Storage Classes */
/**
 * These define where and what the symbol represents
 */
#define C_SMBL_NULL	            0       	/* No entry */
#define C_SMBL_AUTO	            1       	/* Automatic variable */
#define C_SMBL_EXT	            2       	/* External (public) symbol - this covers globals and externs */
#define C_SMBL_STAT	            3       	/* static (private) symbol */
#define C_SMBL_REG	            4       	/* register variable (register # assigned to this variable) */
#define C_SMBL_EXTDEF           5       	/* External definition */
#define C_SMBL_LABEL            6       	/* label */
#define C_SMBL_ULABEL	        7       	/* undefined label */
#define C_SMBL_MOS	            8       	/* member of structure */
#define C_SMBL_ARG	            9       	/* function argument */
#define C_SMBL_STRTAG	        10      	/* structure tag */
#define C_SMBL_MOU	            11      	/* member of union */
#define C_SMBL_UNTAG            12      	/* union tag */
#define C_SMBL_TPDEF            13      	/* type definition */
#define C_SMBL_USTATIC          14      	/* undefined static */
#define C_SMBL_ENTAG            15      	/* enumaration tag */
#define C_SMBL_MOE	            16      	/* member of enumeration */
#define C_SMBL_REGPARM          17      	/* register parameter */
#define C_SMBL_FIELD            18      	/* bit field */
#define C_SMBL_AUTOARG          19      	/* auto argument */
#define C_SMBL_LASTENT          20      	/* dummy entry (end of block) */
#define C_SMBL_BLOCK            100     	/*". bb" or ".eb" - beginning or end of block */
#define C_SMBL_FCN	            101     	/*". bf" or ".ef" - beginning or end of function */
#define C_SMBL_EOS	            102     	/* end of structure */
#define C_SMBL_FILE	            103     	/* file name */
#define C_SMBL_LINE	            104     	/* line number, reformatted as symbol */
#define C_SMBL_ALIAS            105     	/* duplicate tag */
#define C_SMBL_HIDDEN           106     	/* ext symbol in dmert public lib */
#define C_SMBL_EFCN	            255     	/* physical end of function */

#define DEBUG_SHOW_DETAILED 1

#pragma pack(push, 1)
typedef struct {
    u16 magic;          /* FHDR_MAGIC_AMD64 (more exist but we only support these)  */
    u16 nscns;
    i32 timedate;
    i32 symptr;
    i32 nsyms;
    u16 optHdrSize;
    u16 flags;          /* see above */
} fHdr_t;

#define OHDR_MAGIC_10 0x010b
#define OHDR_MAGIC_20 0x020b

typedef struct {
    u16 magic;          /* 0x010b or 0x020b */
    u16 vstamp;
    u32 tsize;
    u32 dsize;
    u32 bsize;
    u32 entry;          /* used for %eip */
    u32 tstart;
    u32 dstart;
} oHdr_t;

typedef struct {
    char name[8];       /** names with length=8 will not have a null-terminated byte.
                            if the name starts with '/', then the following chars are
                            decimal digits representing the offset of the name in
                            the String Table */
    i32 paddr;          /* generally the same value as vaddr */
    i32 vaddr;          /* " "                         paddr */
    i32 size;
    i32 scnptr;         /* file offset to section data */
    i32 relptr;         /* " "          relocation table for this section */
    i32 lnnoptr;        /* " "          line number table   " " */
    u16 nreloc;         /* number of relocation table entries */
    u16 nlnno;          /* " "       line number table entries */
    i32 flags;
} sHdr_t;

typedef struct {
    i32 rvaddr;
    i32 symndx;
    u16 type;       /* implementation-specific. for now, we handle type=6 and type=20 */
} relEntry_t;

typedef struct {
    union {
        i32 symndx;
        i32 paddr;
    } addr;
    u16 lnno;           /* Line number of the symbol in the source file (debug info) */
    /**
     * if lnno = 0, then symndx indicates the function name symbol in the symbol table
     * This 0-lnno entry is followed by additional entries which indicate through
     * p_addr the byte offset into the section where this line starts.
     * 
     * given this, if an exception occurs you can trace it back to a function and a line
     * number with that function. Since each line of each function has its own entry, you
     * can check the lnno table of a symbol with the current offset to find out what line
     * of a function you are on
     * 
     * should not expect this to be after any section other than .text, but COFF allows
     * for *any* section to have a lnno table.
     */
} lnnoEntry_t;

/* Do NOT use sizeof(symEntry_t) when reading from files. Use
    SYMENTRY_FSIZE instead. The debugger may break if you 
    use sizeof(symEntry_t) to calculate file offsets (e.g. strTableOffset) */
typedef struct {
    union {
        char name[8];
        struct {
            u32 zeroes;
            u32 offset; /* You will note the ` -4 ` applied to the offset in load_coff.
                           are the offsets at the BASE of the String Table. Not strTable.blob */
        } packed;
    } meta;
    i32 value;
    i16 scnum;          /** the section number that this symbol belongs to
                            OR
                            a special symbol (see N_SMBL_** definitions above)
                            Note that the section table is 1-indexed (by
                            the format, not internally)*/
    u16 type;           /* See T_SMBL_* and DT_SMBL_* macros above */
    char sclass;        /* storage class */
    char numaux;        /* auxiliary count */
    b32 isaux;
} symEntry_t;

#define SYMENTRY_FSIZE 18

#pragma pack(pop)

/**
 * The overall structure of a COFF file is as follows
 * 
 * |    [file header]
 * v    [?opt header?]
 *      [N section headers]           (.text, .data, .bss, .pdata, .xdata, .rdata, ...)
 * 
 *      . [s1Blob]
 *      . [s2Blob]
 *      . [s3Blob]
 *      ...
 * 
 *      [relocation entries]
 *      [symbol table]
 *      [strings table]
 */

typedef struct {
    char *blob;
    u32 size;
} strTable_t;

/* raw section data blob */
typedef struct {
    u64 size;
    u8* blob;
    lnnoEntry_t* lnnoLUT;
} scnBlob_t;
typedef struct {
    fHdr_t fHdr;
    oHdr_t oHdr;
    sHdr_t* sHdrs;
    scnBlob_t* scns;
    relEntry_t** relocs; /* per-section relocation directives */
    symEntry_t* symTable;
    strTable_t strTable;
    char* testing;
} coff_t;

coff_t load_coff(const char* fpath, arena_t* arena);

#endif