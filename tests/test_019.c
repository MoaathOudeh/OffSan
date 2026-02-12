/* old name: memcpy_full_source_overflow_small_dest.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char dest[4];
    char src[8] = "toolong";
    memcpy(dest, src, sizeof(src));
    return 0;
}
