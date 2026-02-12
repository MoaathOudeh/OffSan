/* old name: argv_strcpy_overflow.c */
// Intentional overflow via argv -> strcpy, should abort under instrumentation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *src = (argc > 1) ? argv[1] : "this_input_is_longer_than_buf";
    char buf[8];

    volatile int marker = 0x1234;
    (void)marker;

    printf("copying \"%s\" into buf[8] (overflow expected)\n", src);
    strcpy(buf, src); // overflow on purpose

    return 0;
}
