/* old name: variadic_formatter_chain_overflow.c */
// Layered variadic formatting that intentionally overruns small buffers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_unbounded(char *dst, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(dst + strlen(dst), fmt, ap); // unchecked growth -> overflow target buffer
    va_end(ap);
}

int main(int argc, char **argv)
{
    char buf[32] = "seed";
    const char *alpha = (argc > 1) ? argv[1] : "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *beta = (argc > 2) ? argv[2] : "0123456789-0123456789";
    const char *gamma = (argc > 3) ? argv[3] : "padding-padding-padding";

    append_unbounded(buf, ":%s:", alpha);
    append_unbounded(buf, "%s::%s", beta, beta);
    append_unbounded(buf, ":%s:%s", gamma, gamma);

    printf("formatted=%s\n", buf);
    return 0;
}
