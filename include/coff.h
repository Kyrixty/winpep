#ifndef COFF_H
#define COFF_H

#include "common.h"
#include "fs.h"
#include <stdlib.h>
                                        /* if set */
#define F_RELFLG    1 << 0      /* There is no relocation information in this file. Usually clear for object files and set for executables */
#define F_EXEC      1 << 1      /* All unresolved symbols have been resolved; this file is executable */
#define F_LNNO      1 << 2      /* Line number information has been removed or omitted */
#define F_LSYMS     1 << 3      /* All local symbols have been removed or omitted */
#define F_AR32WR    1 << 8      /* This file is 32-bit little endian */

#define FHDR_MAGIC_AMD64 0x8664

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
    char name[8];
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
#pragma pack(pop)

typedef struct {
    fHdr_t fHdr;
    oHdr_t oHdr;
    sHdr_t sHdr;
} coff_t;

coff_t load_coff(const char* fpath);

#endif