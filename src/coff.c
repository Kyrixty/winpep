#include "coff.h"
#include <memory.h>

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
    /* Grab section-related blobs from header info */
    for (u32 i = 0; i < coff.fHdr.nscns; i++) {
        coff.scns[i].blob = ALLOC_ARRAY(arena, u8, coff.sHdrs[i].size);
        fseek(f, coff.sHdrs[i].scnptr, SEEK_SET);
        fread(coff.scns[i].blob, 1, coff.sHdrs[i].size, f);
        coff.scns[i].size = coff.sHdrs[i].size;

        long curPos = ftell(f);

        /* relocation directives */
        if (coff.sHdrs[i].nreloc) {
            coff.relocs[i] = ALLOC_ARRAY(arena, relEntry_t, coff.sHdrs[i].nreloc);
            fseek(f, coff.sHdrs[i].relptr, SEEK_SET);
            // coff.relocs[i] is relEntry_t* (so don't use &coff.relocs[i] !)
            fread(coff.relocs[i], sizeof(relEntry_t), coff.sHdrs[i].nreloc, f);

            if (DEBUG_SHOW_RELOC) {
                for (u32 j = 0; j < coff.sHdrs[i].nreloc; j++) {
                    relEntry_t r = coff.relocs[i][j];
                    printf(
                        "\n========\nSection %d ('%s') Reloc %d\nRVADDR: 0x%x\nSYMNDX: %d\nTYPE: 0x%x",
                        i, coff.sHdrs[i].name, j, r.rvaddr, r.symndx, r.type
                    );
                }
            }
        }
    }

    /* Strings Table */
    u64 strTableOffset = coff.fHdr.symptr + coff.fHdr.nsyms * sizeof(symEntry_t);
    fseek(f, strTableOffset, SEEK_SET);
    // the format writes the size of strTable.blob in the first 4
    // bytes at strTableOffset, so we shouldn't rely on the size
    // of the field.
    fread(&coff.strTable.size, STRTABLE_SIZE_SIZE, 1, f);
    coff.strTable.blob = ALLOC_ARRAY(arena, char, coff.strTable.size);
    fread(coff.strTable.blob, 1, coff.strTable.size, f);

    return coff;
}
