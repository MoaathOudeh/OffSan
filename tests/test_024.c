/* old name: negative_index_read_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>

int main(void) {
    char buf[8];
    char *p = buf;
    char leak = p[-1];
    (void)leak;
    return 0;
}
