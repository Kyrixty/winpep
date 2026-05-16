#include <stdio.h>

static int va;
static int vb = 123;
static double aReallyLongName = 5.0;

void my_func(int x) {
    printf("x is %d\n", x);
}

static void my_func_static(int x) {
    printf("x is %d\n", x);
}

int my_other_func(int x) {
    return x + 1;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}