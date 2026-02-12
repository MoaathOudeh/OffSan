/* old name: memmove_overlap_inbounds_no_oob.c */
// Overlapping memmove across regions of the same stack buffer (no OOB)
#include <stdio.h>
#include <string.h>

int main(void)
{
    char buf[32] = "abcdefghijklmnopqrstuvwx";
    memmove(buf + 4, buf, 12); // intentional overlap but within bounds
    buf[16] = '\0';
    printf("memmove overlap result: %s\n", buf);
    return 0;
}
