#include "coff.h"
#include "utils.h"
#include "x86.h"
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

// static char* RELOC_NAME_MAP[] = {
//     "ABSOLUTE",
//     "ADDR64",
//     "ADDR32",
//     [0x4] = "REL32",
//     [0xb] = "SECREL",
// };

// static u8 char_to_num[] = {
//     ['0'] = 0,
//     ['1'] = 1,
//     ['2'] = 2,
//     ['3'] = 3,
//     ['4'] = 4,
//     ['5'] = 5,
//     ['6'] = 6,
//     ['7'] = 7,
//     ['8'] = 8,
//     ['9'] = 9,
//     ['a'] = 10, ['A'] = 10,
//     ['b'] = 11, ['B'] = 11,
//     ['c'] = 12, ['C'] = 12,
//     ['d'] = 13, ['D'] = 13,
//     ['e'] = 14, ['E'] = 14,
//     ['f'] = 15, ['F'] = 15
// };


/**
 * Returns -1 on error (c outside of base bounds or not a digit), 
 * otherwise returns the digit value given a base.
 */
u32 digit_value(char c, u32 base) {
    u32 d = 0;
    if ('0' <= c && c <= '9')
        d = c - '0';
    else if ('a' <= c && c <= 'z')
        d = c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        d = c - 'a' + 10;
    else
        return -1;
    return d < base ? d : -1;
}

/**
 * Parses an integer in `s` with base `base`. 
 * Check outNumParsed to see how many characters
 * in s were parsed (or set it to NULL if you don't
 * care about this value)
 * 
 * *outNumParsed == 0 indicates an error
 * (outNumParsed will be set if at least 1 digit was parsed,
 * otherwise it will retain it's value before this function
 * was called.)
 */
i32 parse_int(char* s, i32 base, u32* outNumParsed) {
    bool any = false;
    i32 val = 0;
    i32 d = 0;
    u32 nParsed = 0;
    i32 sign = 1;
    if (!s) return 0;
    while (isspace(*s)) { nParsed++; s++; }
    if (*s == '-') { nParsed++; sign = -1; }
    while (*s && (d = digit_value(*s++, base)) != -1) {
        val = val * base + d;
        any = true;
        nParsed++;
    }
    if (any && outNumParsed != NULL) {
        *outNumParsed = nParsed;
    }
    return sign * val;
}

const char* get_strtable_at(const coff_t* coff, u32 offset) {
    return coff->strTable.blob + offset - 4; /* See notes on this in strTable_t */
}

#define SCNUM_TO_SCIDX(scnum) (scnum - 1)

const sHdr_t* get_scn_hdr(const coff_t* coff, i32 scnIdx) {
    if (scnIdx <= 0) {
        return &coff->sHdrs[0]; /* ??? */
    }
    return &coff->sHdrs[scnIdx];
}

const scnBlob_t* get_scn_blob(const coff_t* coff, i32 scnIdx) {
    if (scnIdx <= 0) {
        return &coff->scns[0]; /* ??? */
    }
    return &coff->scns[scnIdx];
}

const char* get_scn_name(const coff_t* coff, i32 scnIdx) {
    if (scnIdx <= 0) {
        return coff->sHdrs[0].name; /* ??? */
    }
    sHdr_t hdr = *get_scn_hdr(coff, scnIdx);
    if (hdr.name[0] == '/') {
        i32 offset = parse_int(hdr.name + 1, 10, NULL);
        return get_strtable_at(coff, offset);
    }
    return coff->sHdrs[scnIdx].name;
}

// static i32 __sym_cmp_qsort(const void* a, const void* b) {
//     return ((symEntry_t*)a)->value - ((symEntry_t*)b)->value;
// }

static i32 __fnref_cmp_qsort(const void* a, const void* b) {
    return ((fnRef_t*)a)->offset - ((fnRef_t*)b)->offset;
}

b32 static sym_is_fn(const coff_t* coff, symEntry_t sym) {
    // Unsure if this is correct, may also be slow for many symbols
    return SYMBOL_IS_FCN(sym.type);
}

b32 static fn_sym_has_defn(symEntry_t sym) {
    return sym.isfn && sym.scnum > 0;
} 

symEntry_t* get_fn_symbols(const coff_t* coff, arena_t* arena, u32* outTableSize) {
    u32 nfns = coff->ndfns;
    u32 tableSize = sizeof(symEntry_t) * nfns;

    // Grab functions from symTable
    symEntry_t* tmp = ALLOC_ARRAY(arena, symEntry_t, nfns);
    for (u32 i = 0; i < nfns; i++) {
        symEntry_t sym = coff->symTable[coff->fnRefs[i].symIdx];
        tmp[i] = sym;
    }
    // qsort(
    //     tmp,
    //     nfns,
    //     sizeof(symEntry_t),
    //     __sym_cmp_qsort
    // );
    if (outTableSize) {
        *outTableSize = tableSize;
    }

    return tmp;
}

void print_fns_sorted(const coff_t* coff, const char* ignored, arena_t* arena) {
    // Don't want to sort coff->symTable in place so we need to
    // copy.
    u32 nfns = coff->ndfns;
    u32 tableSize = 0;
    symEntry_t* fnSymbols = get_fn_symbols(coff, arena, &tableSize);
    
    for (u32 i = 0; i < nfns; i++) {
        // Any function with a size of 0 has only been declared
        print_symbol(coff, fnSymbols[i]);
        // printf("0x%x\n", coff->fnRefs[i].fnSize);
    }
    arena_pop(arena, tableSize);
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
    const char* symScnName = get_scn_name(coff, SCNUM_TO_SCIDX(sym.scnum));
    const char* symName = get_symbol_name(coff, &sym);
    char* padding = (!symScnName || strlen(symScnName) > 8) ? "\t" : "\t\t";
    printf("%s\t%d%s(%s)\t0x%x\t%s\n",
        symScnName,
        sym.scnum,
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

void print_str_table(const coff_t* coff, const char* ignored, arena_t* _ignored) {
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

void print_symbol_table(const coff_t* coff, const char* ignored, arena_t* _ignored) {
    printf("\n====SYMBOL TABLE NAMES====\n");
    printf("Indx\tSctn\tSnNo\t\tStrg\t\tValu\tName\n");
    for (u32 i = 0; i < coff->fHdr.nsyms; i++) {
        symEntry_t sym = coff->symTable[i];
        printf("[%d]\t", i);
        print_symbol(coff, sym);
    }
    printf("\n====END SYMBOL TABLE NAMES====\n");
}

void print_all_relocs(const coff_t* coff, const char* ignored, arena_t* _ignored) {
    for (u32 i = 0; i < coff->fHdr.nscns; i++) {
        for (u32 j = 0; j < coff->sHdrs[i].nreloc; j++) {
            relEntry_t r = coff->relocs[i][j];
            print_reloc(coff, i, j, r);
        }
    }
}

void print_scn(const coff_t* coff, i32 scnIdx) {
    sHdr_t sHdr = coff->sHdrs[scnIdx];
    const char* scn_name = get_scn_name(coff, scnIdx);
    printf("0x%x\t0x%x\t0x%x\t0x%x\t%s\n",
        sHdr.scnptr,
        sHdr.size,
        sHdr.vaddr,
        sHdr.lnnoptr,
        scn_name);
}

void print_all_scns(const coff_t* coff, const char* ignored, arena_t* _ignored) {
    printf("===== BEGIN SECTIONS =====\n");
    printf("Indx\tOffset\tSize\tVAddr\tLnnoptr\tName\n");
    for (i32 i = 0; i < coff->fHdr.nscns; i++) {
        printf("[%d]\t", i);
        print_scn(coff, i);
    }
    printf("===== END SECTIONS =====\n");
}

/**
 * May want to just return a buffer, but for now, printing is fine
 */
static u8* __hexdump(const coff_t* coff, u32 offset, u32 size, arena_t* arena) {
    if (offset + size >= coff->fileLen) {
        printf("hexdump: offset + size must be within file bounds.\n");
        return NULL;
    }
    u8* buf = ALLOC_ARRAY(arena, u8, size);
    memcpy(buf, coff->fileBlob + offset, size);
    return buf;
}

static void __print_hexdump(const coff_t* coff, u32 colsPerRow, u32 offset, u32 size) {
    if (offset + size >= coff->fileLen) {
        printf("hexdump: offset + size must be within file bounds.\n");
        return;
    }
    u32 i = offset;
    printf("            \t");
    for (u32 k = 0; k < colsPerRow; k++) {
        printf("%02x ", k);
    }
    printf("\t\tDecoded Text\n");
    while (i < offset + size) {
        printf("<0x%08x>\t", i);
        for (u32 j = 0; j < colsPerRow; j++) {
            if ((i + j) < offset + size)
                printf("%02x ", coff->fileBlob[i + j] & 0xff);
            else
                printf("   ");
        }
        printf("\t\t");
        for (u32 j = 0; j < colsPerRow; j++) {
            if ((i + j) < offset + size) {
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
}

void hexdump(const coff_t* coff, const char* query, arena_t* arena) {
    u32 offset = 0;
    u32 size = 0;
    u32 colsPerRow = 0;
    i32 matched = sscanf(query, "hexdump %d 0x%x 0x%x\n", &colsPerRow, &offset, &size);
    i32 isFnDump = str_startswith(query, "hexdump fns", 11);
    if (!isFnDump && (matched != 3 || colsPerRow == 0)) {
        printf("Usage:\thexdump [COLSPERROW > 0] OFFSET_HEX SIZE_HEX\n\thexdump fns\n");
        printf("Example: hexdump 5 0xffe 0xff => prints 0xff (5 bytes per row of output) bytes as hex starting at offset 0xff in the COFF file\n");
        printf("Example: hexdump fns => Prints hexdump of all function symbols in the file.\n");
        return;
    }

    if (!isFnDump) { 
        printf("==== BEGIN HEXDUMP @0x%x:0x%x =====\n",
            offset,
            offset + size);
            __print_hexdump(coff, colsPerRow, offset, size);
        printf("==== END HEXDUMP @0x%x:0x%x =====\n",
            offset,
            offset + size);
    }
    else {
        u32 tableSize = 0;
        symEntry_t* fnSymbols = get_fn_symbols(coff, arena, &tableSize);
        printf("==== BEGIN HEXDUMP @FUNCTIONS ====\n");
        for (u32 i = 0; i < coff->ndfns; i++) {
            symEntry_t sym = fnSymbols[i];
            fnRef_t fnRef = coff->fnRefs[i];
            sHdr_t sHdr = *get_scn_hdr(coff, SCNUM_TO_SCIDX(sym.scnum));
            u32 offset = sHdr.scnptr + sym.value;
            u32 size = fnRef.fnSize;
            printf("%s:\n", get_symbol_name(coff, &sym));
            __print_hexdump(coff, 16, offset, size);
        }
        printf("==== END HEXDUMP @FUNCTIONS ====\n");
    }
    
}

void disasm_fn(const coff_t* coff, const char* query, arena_t* arena) {
    symEntry_t* fnSyms = get_fn_symbols(coff, arena, NULL);
    char fName[128];
    i32 matched = sscanf(query, "disasm %127s", fName);
    if (matched != 1) {
        printf("Usage:\tdisasm <func_name>\n");
        printf("Example: disasm printf\n");
        printf("Note: function names may not exceed 127 characters in length.\n");
        return;
    }
    fnRef_t matchedRef = {0};
    sHdr_t matchedScnHdr = {0};
    b32 found = 0;
    for (u32 i = 0; i < coff->ndfns; i++) {
        symEntry_t fnSym = fnSyms[i];
        const char* fnSymName = get_symbol_name(coff, &fnSym);
        if (str_eq(fnSymName, fName, 127)) {
            found = true;
            matchedRef = coff->fnRefs[i];
            matchedScnHdr = *get_scn_hdr(coff, SCNUM_TO_SCIDX(fnSym.scnum));
            break;
        }
    }
    if (!found) {
        printf("disasm: '%s' is not a function.\n", fName);
        return;
    }
    u8* fnBlob = __hexdump(coff, matchedScnHdr.scnptr + matchedRef.offset, matchedRef.fnSize, arena);
    u32 nInstrs = 0;
    x86Instr_t* instrs = disassemble(arena, fnBlob, matchedRef.fnSize, &nInstrs);
    if (instrs) {
        for (u32 i = 0; i < nInstrs; i++) {
            print_x86(instrs[i]);
        }
        arena_pop(arena, sizeof(x86Instr_t) * nInstrs);
    }
}

coff_t load_coff(const char* fpath, arena_t* arena) {
    coff_t coff = {0};
    FILE *f = open_file(fpath, "rb");
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    fseek(f, 0, SEEK_SET);
    coff.fileBlob = ALLOC_ARRAY(arena, u8, flen);
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
    // u64 sHdrsEndOffset = ftell(f);
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
    char currNumaux = 0;
    u32 nfns = 0;
    u32 ndfns = 0;
    for (u32 i = 0; i < coff.fHdr.nsyms; i++) {
        fread(&coff.symTable[i], SYMENTRY_FSIZE, 1, f);
        symEntry_t sym = coff.symTable[i];
        if (sym_is_fn(&coff, sym)) {
            coff.symTable[i].isfn = true;
            nfns++;
            if (fn_sym_has_defn(coff.symTable[i]))
                ndfns++;
        }
        if (sym.numaux) {
            currNumaux = sym.numaux;
            continue;
        }
        if (currNumaux) {
            coff.symTable[i].isaux = true;
            currNumaux--;
        }
        
    }
    // setup function symbol indexes
    coff.ndfns = ndfns;
    coff.nufns = nfns - ndfns;
    coff.fnRefs = ALLOC_ARRAY(arena, fnRef_t, ndfns);
    coff.fnRefsUndefined = ALLOC_ARRAY(arena, fnRef_t, coff.nufns);
    u32 j = 0;
    u32 k = 0;
    for (u32 i = 0; i < coff.fHdr.nsyms; i++) {
        if (coff.symTable[i].isfn) {
            b32 hasDefn = fn_sym_has_defn(coff.symTable[i]);
            if (hasDefn) {
                coff.fnRefs[j++] = (fnRef_t) {
                    .symIdx = i,
                    .hasDefn = hasDefn,
                    .fnSize = 0,
                    .offset = coff.symTable[i].value};
            }
            else {
                coff.fnRefsUndefined[k++] = (fnRef_t) {
                    .symIdx = i,
                    .hasDefn = hasDefn,
                    .fnSize = 0,
                    .offset = 0};
            }
        }

    }

    qsort(coff.fnRefs, ndfns, sizeof(fnRef_t), __fnref_cmp_qsort);

    // finish function symbol setup
    for (u32 i = 0; i < ndfns; i++) {
        u32 fnSize = 0;
        symEntry_t curr = coff.symTable[coff.fnRefs[i].symIdx], next;
        if (i < ndfns - 1) {
            next = coff.symTable[coff.fnRefs[i + 1].symIdx];
            fnSize = next.value - curr.value;
        }
        else {
            sHdr_t fnScnHdr = *get_scn_hdr(&coff, SCNUM_TO_SCIDX(curr.scnum));
            fnSize = fnScnHdr.size - curr.value;
        }
        coff.fnRefs[i].fnSize = fnSize;
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
