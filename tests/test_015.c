/* old name: int_buffer_index_past_end_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    int numbers[4];
    int *p = numbers;
    p[4] = 123;
    return 0;
}
