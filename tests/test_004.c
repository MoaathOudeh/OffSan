/* old name: argv_sized_vla_overwrite_overflow.c */
// VLA size driven by argv, then over-written with 2x length
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int n = (argc > 1) ? atoi(argv[1]) : 8;
    if (n <= 0)
        n = 8;
    if (n > 64)
        n = 64;

    char buf[n];
    printf("vla size=%d; writing %d bytes\n", n, 2 * n);
    memset(buf, 'C', (size_t)(2 * n)); // overflow when n>0
    return 0;
}
