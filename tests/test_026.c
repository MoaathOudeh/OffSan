/* old name: one_past_end_byte_write_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char buf[8];
    buf[8] = 'X';
    return 0;
}
