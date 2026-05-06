#include <stdio.h>
#include "coff.h"

int main() {
    coff_t coff = load_coff("./build/sample.o");
    return 0;
}