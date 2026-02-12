/* old name: memset_past_buffer_end_overflow.c */
// memset past the end of a small stack buffer
#include <stdio.h>
#include <string.h>

int main(void)
{
    char buf[8];
    memset(buf, 'A', sizeof(buf) + 16); // intentional overflow
    puts("memset done");
    return 0;
}
