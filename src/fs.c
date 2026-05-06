#include "fs.h"

FILE* open_file(const char* fpath, const char* mode) {
    FILE *f = fopen(fpath, mode);
    if (!f) {
        fprintf(stderr, "Could not open '%s' (mode='%s').\n", fpath, mode);
        exit(1);
    }
    return f;
}