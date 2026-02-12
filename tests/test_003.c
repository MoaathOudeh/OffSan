/* old name: argv_length_driven_memcpy_overflow.c */
// argv-provided length used directly in memcpy -> overflow expected
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *src = (argc > 1) ? argv[1] : "overflow-from-argv";
    size_t len = strlen(src);
    char buf[8];

    printf("memcpy %zu bytes into buf[8]\n", len);
    memcpy(buf, src, len); // overflow on long argv

    return 0;
}
