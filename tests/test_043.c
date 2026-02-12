/* old name: strncpy_long_source_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char small[4];
    strncpy(small, "0123456789", 10);
    return 0;
}
