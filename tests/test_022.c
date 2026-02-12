/* old name: memset_over_large_length_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char dest[4];
    memset(dest, 0xff, 16);
    return 0;
}
