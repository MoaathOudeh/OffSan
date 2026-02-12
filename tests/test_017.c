/* old name: long_argv_clamped_copy_no_oob.c */
// Long argv strings but clamped copies (no OOB)
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[40];
    const char *input = (argc > 1) ? argv[1] : "a-very-long-default-string-input";

    size_t n = strlen(input);
    if (n >= sizeof(buf))
        n = sizeof(buf) - 1;
    memcpy(buf, input, n);
    buf[n] = '\0';

    printf("safe argv: %s (%zu)\n", buf, n);
    return 0;
}
