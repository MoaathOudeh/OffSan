/* old name: oversized_vla_tail_write_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    int n = 600;
    char big[n];
    big[n] = '#';
    return 0;
}
