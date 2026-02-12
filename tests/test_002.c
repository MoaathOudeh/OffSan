/* old name: argv_copy_clamped_no_oob.c */
// Argv handling with explicit bounds checks (no overflow)
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *src = (argc > 1) ? argv[1] : "default-argv";
    char buf[32];

    size_t n = strlen(src);
    if (n >= sizeof(buf))
        n = sizeof(buf) - 1;

    memcpy(buf, src, n);
    buf[n] = '\0';

    printf("argv copied safely: %s (len=%zu)\n", buf, n);
    return 0;
}
