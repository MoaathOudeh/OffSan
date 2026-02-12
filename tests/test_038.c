/* old name: sprintf_into_too_small_buffer_overflow.c */
// sprintf into undersized buffer
#include <stdio.h>
#include <string.h>

int main(void)
{
    char buf[12];
    const char *part = "0123456789ABCDEF";
    sprintf(buf, "x-%s", part); // overflow expected
    printf("%s\n", buf);
    return 0;
}
