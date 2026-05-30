#include "coff.h"
#include <memory.h>
#include <string.h>

/* Size of the size of the str table */
#define STRTABLE_SIZE_SIZE 4

static char* STORAGE_NAME_MAP[] = {
"C_SMBL_NULL",	   
"C_SMBL_AUTO",	   
"C_SMBL_EXT",	   
"C_SMBL_STAT",	   
"C_SMBL_REG",	   
"C_SMBL_EXTDEF",  
"C_SMBL_LABEL",   
"C_SMBL_ULABEL",
"C_SMBL_MOS",	   
"C_SMBL_ARG",	   
"C_SMBL_STRTAG",
"C_SMBL_MOU",	   
"C_SMBL_UNTAG",   
"C_SMBL_TPDEF",   
"C_SMBL_USTATIC", 
"C_SMBL_ENTAG",   
"C_SMBL_MOE",	   
"C_SMBL_REGPARM", 
"C_SMBL_FIELD",   
"C_SMBL_AUTOARG", 
"C_SMBL_LASTENT", 
[100] = "C_SMBL_BLOCK",   
[101] = "C_SMBL_FCN",	   
[102] = "C_SMBL_EOS",	   
[103] = "C_SMBL_FILE",	   
[104] = "C_SMBL_LINE",	   
[105] = "C_SMBL_ALIAS",   
[106] = "C_SMBL_HIDDEN",  
[255] = "C_SMBL_EFCN",	   
};

static char* RELOC_NAME_MAP[] = {
    "ABSOLUTE",
    "ADDR64",
    "ADDR32",
    [0x4] = "REL32",
    [0xb] = "SECREL",
};

/**
 * Sums all numeric chars in s (0-9)
 * Other characters are ignored.
 */
i32 parse_int(char* s) {
    int x = 0;
    if (!s) return x;
    while (*s) {
        if ('0' <= *s && '9' >= *s) {
            x *= 10;
            x += (*s - '0');
        }
        s++;
    }
    return x;
}

static u8 char_to_num[] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15
};

/**
 * Parses a number in `s` with base `base`. 
 * Note that it does not respect the range of values
 * possible in a given base: parse_int("f", 8) will return 15.
 */
i32 parse_num(char* s, i32 base) {
    int x = 0;
    if (!s) return x;
    while (*s) {
        x *= base;
        if (WITHIN_RANGE(*s, '0', '9'))
            x += char_to_num[*s];
        if (WITHIN_RANGE(*s, 'a', 'f'))
            x += char_to_num[*s];
        if (WITHIN_RANGE(*s, 'A', 'F'))
            x += char_to_num[*s];
        s++;
    }
    return x;
}

char* get_strtable_at(const coff_t* coff, u32 offset) {
    return coff->strTable.blob + offset - 4; /* See notes on this in strTable_t */
}

char* get_scn_name(const coff_t* coff, i32 scnIdx) {
    if (scnIdx <= 0) {
        return coff->sHdrs[0].name; /* ??? */
    }
    sHdr_t hdr = coff->sHdrs[scnIdx];
    if (hdr.name[0] == '/') {
        i32 offset = parse_int(hdr.name);
        return get_strtable_at(coff, offset);
    }
    return coff->sHdrs[scnIdx - 1].name;
}

static const char* AUX_NAME = "<auxiliary symbol>";

const char* get_symbol_name(const coff_t* coff, const symEntry_t* sym) {
    if (sym->isaux)
        return AUX_NAME;
    if (sym->meta.packed.zeroes != 0) {
        return sym->meta.name;
    }
    return get_strtable_at(coff, sym->meta.packed.offset);
}

void print_symbol(const coff_t* coff, symEntry_t sym) {
    char* symScnName = get_scn_name(coff, sym.scnum);
    const char* symName = get_symbol_name(coff, &sym);
    char* padding = (!symScnName || strlen(symScnName) > 8) ? "\t" : "\t\t";
    printf("%s%s(%s)\t0x%x\t%s\n",
        symScnName,
        padding,
        STORAGE_NAME_MAP[sym.sclass],
        sym.value,
        symName);
}

void print_reloc(const coff_t* coff, u32 scnIdx, u32 relIdx, relEntry_t rel) {
    printf(
        "\n========\nSection %d ('%s') Reloc %d\nRVADDR: 0x%x\nSYMBOL: %s\nTYPE: 0x%x",
        scnIdx,
        get_scn_name(coff, scnIdx),
        relIdx,
        rel.rvaddr,
        get_symbol_name(coff, &coff->symTable[rel.symndx]),
        rel.type
    );
}

void print_str_table(const coff_t* coff, const char* ignored) {
    const strTable_t* strTable = &coff->strTable;
    printf("\n====STRINGS TABLE====\n");
    for (u32 i = 0;
        i < strTable->size;
        i += strlen(strTable->blob + i) + 1) {
            if (strTable->blob[i]) {
                printf("%s\n", strTable->blob + i);
            }
    }
    printf("\n====END STRINGS TABLE====\n");
}

void print_symbol_table(const coff_t* coff, const char* ignored) {
    printf("\n====SYMBOL TABLE NAMES====\n");
    printf("Indx\tSctn\t\tStrg\t\tValu\tName\n");
    for (u32 i = 0; i < coff->fHdr.nsyms; i++) {
        symEntry_t sym = coff->symTable[i];
        printf("[%d]\t", i);
        print_symbol(coff, sym);
    }
    printf("\n====END SYMBOL TABLE NAMES====\n");
}

void print_all_relocs(const coff_t* coff, const char* ignored) {
    for (u32 i = 0; i < coff->fHdr.nscns; i++) {
        for (u32 j = 0; j < coff->sHdrs[i].nreloc; j++) {
            relEntry_t r = coff->relocs[i][j];
            print_reloc(coff, i, j, r);
        }
    }
}

void print_scn(const coff_t* coff, i32 scnIdx) {
    sHdr_t sHdr = coff->sHdrs[scnIdx];
    char* scn_name = get_scn_name(coff, scnIdx);
    printf("0x%x\t0x%x\t0x%x\t0x%x\t%s\n",
        sHdr.scnptr,
        sHdr.size,
        sHdr.vaddr,
        sHdr.lnnoptr,
        scn_name);
}

void print_all_scns(const coff_t* coff, const char* ignored) {
    printf("===== BEGIN SECTIONS =====\n");
    printf("Indx\tOffset\tSize\tVAddr\tLnnoptr\tName\n");
    for (i32 i = 0; i < coff->fHdr.nscns; i++) {
        printf("[%d]\t", i);
        print_scn(coff, i);
    }
    printf("===== END SECTIONS =====\n");
}

void hexdump(const coff_t* coff, const char* query) {
    u32 offset = 0;
    u32 size = 0;
    u32 colsPerRow = 0;
    i32 matched = sscanf(query, "hexdump %d 0x%x 0x%x\n", &colsPerRow, &offset, &size);
    if (matched != 3) {
        printf("Usage: hexdump COLSPERROW OFFSET_HEX SIZE_HEX\n");
        printf("Example: hexdump 5 0xffe 0xff => prints 0xff (5 bytes per row of output) bytes as hex starting at offset 0xff in the COFF file\n");
        return;
    }
    if (offset + size >= coff->fileLen) {
        printf("hexdump: offset + size must be within file bounds.\n");
        return;
    }


    printf("==== BEGIN HEXDUMP @0x%x:0x%x =====\n",
        offset,
        offset + size);
    u32 i = offset;
    printf("            \t");
    for (u32 k = 0; k < colsPerRow; k++) {
        printf("%02x ", k);
    }
    printf("\t\tDecoded Text\n");
    while (i < offset + size) {
        printf("<0x%08x>\t", i);
        for (u32 j = 0; j < colsPerRow; j++) {
            if ((i + j) < coff->fileLen)
                printf("%02x ", coff->fileBlob[i + j] & 0xff);
            else
                printf("   ");
        }
        printf("\t\t");
        for (u32 j = 0; j < colsPerRow; j++) {
            if ((i + j) < coff->fileLen) {
                char c = coff->fileBlob[i + j];
                if (PRINTABLE(c))
                    printf("%c ", c);
                else
                    printf(". ");
            }
        }
        printf("\n");
        i += colsPerRow;
    }
    printf("==== END HEXDUMP @0x%x:0x%x =====\n",
        offset,
        offset + size);
}

coff_t load_coff(const char* fpath, arena_t* arena) {
    coff_t coff = {0};
    FILE *f = open_file(fpath, "rb");
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    fseek(f, 0, SEEK_SET);
    coff.fileBlob = ALLOC_ARRAY(arena, char, flen);
    fread(coff.fileBlob, 1, flen, f);
    coff.fileLen = flen;
    long beforePos = 0;
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
    /* Warning: processing sections prior to loading the section headers
    may yield weird behaviour. Unless you're confident you need to, it's
    best to process parts of the file after section headers have been read. */


    /* Strings Table */
    beforePos = ftell(f);
    u64 strTableOffset = coff.fHdr.symptr + coff.fHdr.nsyms * SYMENTRY_FSIZE;
    fseek(f, strTableOffset, SEEK_SET);
    // the format writes the size of strTable.blob in the first 4
    // bytes at strTableOffset, so we shouldn't rely on the size
    // of the field.
    fread(&coff.strTable.size, STRTABLE_SIZE_SIZE, 1, f);
    coff.strTable.blob = ALLOC_ARRAY(arena, char, coff.strTable.size);
    fread(coff.strTable.blob, 1, coff.strTable.size, f);
    
    fseek(f, beforePos, SEEK_SET);

    /* Symbol Table */
    beforePos = ftell(f);
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

    fseek(f, beforePos, SEEK_SET);

    
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
        }

        /* line number debug info */
        if (coff.sHdrs[i].lnnoptr) {
            coff.scns[i].lnnoLUT = ALLOC_ARRAY(arena, lnnoEntry_t, coff.sHdrs[i].nlnno);
            fseek(f, coff.sHdrs[i].lnnoptr, SEEK_SET);
            fread(coff.scns[i].lnnoLUT, sizeof(lnnoEntry_t), coff.sHdrs[i].nlnno, f);
        }
    }

    fclose(f);
    
    return coff;
}
