#include <stdio.h>
#include "pe.h"

int main() {
    b8 is_image;
    win_pe_t pe = load_pe_from_file("build/sample.exe", &is_image);
    return 0;
}