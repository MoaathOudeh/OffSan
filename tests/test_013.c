/* old name: helper_writes_past_caller_overflow.c */
// Cross-function overflow: helper writes too much into caller's buffer
#include <stdio.h>
#include <string.h>

static void writer(char *dst)
{
    char pattern[64];
    memset(pattern, 'B', sizeof(pattern));
    strcpy(dst, pattern); // overflow caller buffer
}

int main(void)
{
    char small[16];
    writer(small);
    puts("writer returned");
    return 0;
}
