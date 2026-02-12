/* old name: large_buffer_terminal_byte_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char big[512];
    big[512] = '!';
    return 0;
}
