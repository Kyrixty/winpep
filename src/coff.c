#include "coff.h"
#include <memory.h>

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
    for (u32 i = 0; i < coff.fHdr.nscns; i++) {
        coff.scns[i].blob = ALLOC_ARRAY(arena, u8, coff.sHdrs[i].size);
        fseek(f, coff.sHdrs[i].scnptr, SEEK_CUR);
        fread(coff.scns[i].blob, 1, coff.sHdrs[i].size, f);
        coff.scns[i].size = coff.sHdrs[i].size;
    }

    /* Strings Table */
    u64 strTableOffset = coff.fHdr.symptr + coff.fHdr.nsyms * sizeof(symEntry_t);
    fseek(f, strTableOffset, SEEK_SET);
    // the format writes the size of strTable.blob in the first 4
    // bytes at strTableOffset, so we shouldn't rely on the size
    // of the field.
    fread(&coff.strTable.size, 4, 1, f);
    coff.strTable.blob = ALLOC_ARRAY(arena, char, coff.strTable.size);
    fread(coff.strTable.blob, 1, coff.strTable.size, f);

    return coff;
}
