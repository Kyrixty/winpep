#include <stdio.h>
#include "pe.h"

win_pe_t load_pe_from_file(const char* fpath, b8* OUT_is_image) {
    FILE *f = fopen(fpath, "rb");
    if (!f) {
        fprintf(stderr, "Could not open path '%s'.\n", fpath);
        exit(1);
    }
    win_pe_t pe = {0};
    // Determine if file is image (if it is then PE32/PE32+) or object
    u16 magic = 0;
    fread(&magic, sizeof(u16), 1, f);
    fseek(f, 0, SEEK_SET);
    if (magic == 0x5a4d) { // image
        *OUT_is_image = true;
        fread(&pe.win_pe_img, sizeof(win_pe_img_t), 1, f);
    } else { // unsupported (COFF file?)
        perror("Unsupported file type.\n");
    }
}