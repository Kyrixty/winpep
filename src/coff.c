#include "coff.h"
#include <memory.h>
#include <string.h>

/* Size of the size of the str table */
#define STRTABLE_SIZE_SIZE 4

coff_t load_coff(const char* fpath, arena_t* arena) {
    coff_t coff = {0};
    FILE *f = open_file(fpath, "rb");
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    fseek(f, 0, SEEK_SET);
    fread(&coff.fHdr.magic, sizeof(coff.fHdr.magic), 1, f);

    // headers
    if (coff.fHdr.magic != FHDR_MAGIC_AMD64) {
        fprintf(stderr, "Cannot load file '%s' as it is not a COFF file (magic = %x)", fpath, coff.fHdr.magic);
        exit(1);
    }
    fseek(f, 0, SEEK_SET);
    fread(&coff.fHdr, sizeof(coff.fHdr), 1, f);

    if (coff.fHdr.optHdrSize != 0) {
        // optional header present in file
        fread(&coff.oHdr, sizeof(coff.oHdr), 1, f);
        if (coff.oHdr.magic != OHDR_MAGIC_10 && coff.oHdr.magic != OHDR_MAGIC_20) {
            fprintf(stderr, "Cannot load file '%s' due to invalid optional header (magic = %x)", fpath, coff.oHdr.magic);
            exit(1);
        }
    }

    coff.sHdrs = ALLOC_ARRAY(arena, sHdr_t, coff.fHdr.nscns);
    fread(coff.sHdrs, sizeof(sHdr_t), coff.fHdr.nscns, f);
    u64 sHdrsEndOffset = ftell(f);

    coff.scns = ALLOC_ARRAY(arena, scnBlob_t, coff.fHdr.nscns);
    coff.relocs = ALLOC_ARRAY(arena, relEntry_t*, coff.fHdr.nscns);
    /* Grab section-related blobs via section header offsets */
    for (u32 i = 0; i < coff.fHdr.nscns; i++) {
        coff.scns[i].blob = ALLOC_ARRAY(arena, u8, coff.sHdrs[i].size);
        fseek(f, coff.sHdrs[i].scnptr, SEEK_SET);
        fread(coff.scns[i].blob, 1, coff.sHdrs[i].size, f);
        coff.scns[i].size = coff.sHdrs[i].size;

        /* relocation directives */
        if (coff.sHdrs[i].nreloc) {
            coff.relocs[i] = ALLOC_ARRAY(arena, relEntry_t, coff.sHdrs[i].nreloc);
            fseek(f, coff.sHdrs[i].relptr, SEEK_SET);
            // coff.relocs[i] is relEntry_t* (so don't use &coff.relocs[i] !)
            fread(coff.relocs[i], sizeof(relEntry_t), coff.sHdrs[i].nreloc, f);

            if (DEBUG_SHOW_DETAILED) {
                for (u32 j = 0; j < coff.sHdrs[i].nreloc; j++) {
                    relEntry_t r = coff.relocs[i][j];
                    printf(
                        "\n========\nSection %d ('%s') Reloc %d\nRVADDR: 0x%x\nSYMNDX: %d\nTYPE: 0x%x",
                        i, coff.sHdrs[i].name, j, r.rvaddr, r.symndx, r.type
                    );
                }
            }
        }

        /* line number debug info */
        if (coff.sHdrs[i].lnnoptr) {
            coff.scns[i].lnnoLUT = ALLOC_ARRAY(arena, lnnoEntry_t, coff.sHdrs[i].nlnno);
            fseek(f, coff.sHdrs[i].lnnoptr, SEEK_SET);
            fread(coff.scns[i].lnnoLUT, sizeof(lnnoEntry_t), coff.sHdrs[i].nlnno, f);
        }
    }
    /* Strings Table */
    u64 strTableOffset = coff.fHdr.symptr + coff.fHdr.nsyms * SYMENTRY_FSIZE;
    fseek(f, strTableOffset, SEEK_SET);
    // the format writes the size of strTable.blob in the first 4
    // bytes at strTableOffset, so we shouldn't rely on the size
    // of the field.
    fread(&coff.strTable.size, STRTABLE_SIZE_SIZE, 1, f);
    coff.strTable.blob = ALLOC_ARRAY(arena, char, coff.strTable.size);
    fread(coff.strTable.blob, 1, coff.strTable.size, f);
    if (DEBUG_SHOW_DETAILED) {
        printf("\n====STRINGS TABLE====\n");
        for (u32 i = 0;
            i < coff.strTable.size;
            i += strlen(coff.strTable.blob + i) + 1) {
                if (coff.strTable.blob[i]) {
                    printf("%s\n", coff.strTable.blob + i);
                }
        }
        printf("\n====END STRINGS TABLE====\n");
    }

    /* Symbol Table */
    coff.symTable = ALLOC_ARRAY(arena, symEntry_t, coff.fHdr.nsyms);
    fseek(f, coff.fHdr.symptr, SEEK_SET);
    char currNumaux;
    for (u32 i = 0; i < coff.fHdr.nsyms; i++) {
        fread(&coff.symTable[i], SYMENTRY_FSIZE, 1, f);
        symEntry_t sym = coff.symTable[i];
        if (sym.numaux) {
            currNumaux = sym.numaux;
            continue;
        }
        if (currNumaux) {
            coff.symTable[i].isaux = true;
            currNumaux--;
        }
    }
    if (DEBUG_SHOW_DETAILED) {
        printf("\n====SYMBOL TABLE NAMES====\n");
        for (u32 i = 0; i < coff.fHdr.nsyms; i++) {
            symEntry_t sym = coff.symTable[i];
            if (sym.isaux)
                printf("[%d] <auxiliary symbol>\n", i);
            else if (sym.meta.packed.zeroes != 0) {
                printf("[%d] %s\n", i, sym.meta.name);
            } else {
                printf("[%d] %s\n", i, coff.strTable.blob + sym.meta.packed.offset - 4);
            }
        }
        printf("\n====END SYMBOL TABLE NAMES====\n");
    }


    
    return coff;
}
