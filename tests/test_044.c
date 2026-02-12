/* old name: struct_field_tail_byte_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>

struct Pair {
    int x;
    double y;
};

int main(void) {
    struct Pair pair = {0, 0.0};
    char *bytes = (char *)&pair.y;
    bytes[sizeof(double)] = 0x7f;
    return 0;
}
