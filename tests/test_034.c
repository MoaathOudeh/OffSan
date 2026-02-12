/* old name: pointer_offset_far_oob_write.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>

int main(void) {
    char big[512];
    char *p = big + 400;
    p[200] = '?';
    return 0;
}
