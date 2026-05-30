#include <stdio.h>

static int va;
static int vb = 123;
static double aReallyLongName = 5.0;
int externallyAccessible = 40;
extern int externallyDefined;

void my_func(int x) {
    static int staticInNonStatic = 12;
    int nonStaticInsideNonStatic = 13;
    printf("x is %d\n", x);
}

static void my_func_static(int x) {
    static int staticInsideStatic = 14;
    int nonStaticInsideStatic = 15;
    printf("x is %d\n", x);
    printf("externallyDefined is %d\n", externallyDefined);
}

int my_other_func(int x) {
    return x + 1;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}