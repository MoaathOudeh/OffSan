/* old name: vla_tail_write_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    int n = 6;
    char vla[n];
    vla[n] = '!';
    return 0;
}
