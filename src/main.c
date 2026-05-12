#include <stdio.h>
#include "coff.h"
#include "arena.h"

int main() {
    arena_t* arena = arena_init(MB(64));
    coff_t coff = load_coff("./build/sample.o", arena);

    return 0;
}