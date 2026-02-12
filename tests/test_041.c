/* old name: strcpy_into_tiny_buffer_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char small[4];
    strcpy(small, "toolong");
    return 0;
}
