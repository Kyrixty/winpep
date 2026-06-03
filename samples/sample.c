#include <stdio.h>

static int va;
static int vb = 123;
static double aReallyLongName = 5.0;
int externallyAccessible = 40;
extern int externallyDefined;
/* Unless you specify the pointer as const
(i.e. T* const var) then only the data will be
considered const (and therefore held in rdata.)

Essentially, a string literal will always be in .rdata.
However, the variable aliasing it may not (dataStr, alsoDataStr)
UNLESS that variable is also marked const AND is global.*/
static const char* dataStr = "Hello, rdata (from data)!";
static const char const* alsoDataStr = "Hello, rdata (from data)2!";
static const char* const rdataStr  = "Hello, rdata (from rdata)!";

void my_func(int x) {
    /**
     * Any static variable will be put in .data
     * since it must remember it's value from the previous call.
     * However, you cannot access it outside of my_func (without
     * storing a pointer to it) and thus it is sort of a scoped
     * static variable.
     */
    static int staticInNonStatic = 12;
    int nonStaticInsideNonStatic = 13;
    /**
     * Because constNotInData is scoped to my_func's lifetime, it does
     * not need to exist for the entire lifetime of the program and thus
     * is not in .data (in fact, there is no symbol for constNotInData.)
     */
    char* const constNotInData = "This literal is also in .rdata despite not being marked as const";
    printf("x is %d\n", x); // this string has the same address in .rdata as @x
}

static void my_func_static(int x) {
    static int staticInsideStatic = 14;
    int nonStaticInsideStatic = 15;
    printf("x is %d\n", x); // @x
    printf("externallyDefined is %d\n", externallyDefined);
}

int my_other_func(int x) {
    return x + 1;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}