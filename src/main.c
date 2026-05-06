#include <stdio.h>
#include <string.h>
#include "coff.h"
#include "arena.h"

int main() {
    arena_t* arena = arena_init(MB(64));
    coff_t coff = load_coff("./build/sample.o", arena);

    // String Table
    for (u32 i = 0;
        i < coff.strTable.size;
        i += strlen(coff.strTable.blob + i) + 1) {
            printf("%s\n", coff.strTable.blob + i);
    }
    return 0;
}