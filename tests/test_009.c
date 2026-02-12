/* old name: delta_computed_cross_buffer_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>

int main(void) {
    char a[8] = {0};
    char b[8] = {0};
    int delta;
    char *first;
    char *second;

    if (a > b) {
        delta = a - b;
        first = b;
        second = a;
    } else {
        delta = b - a;
        first = a;
        second = b;
    }

    printf("delta = %d\n", delta);
    first[delta+2] = 'X';
    return 0;
}
