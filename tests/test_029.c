/* old name: packet_inline_payload_overflow.c */
// Overflow the inline payload of a struct Packet (from code.c idea)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Packet
{
    char buf[8];
    int canary;
};

int main(int argc, char **argv)
{
    const char *payload = (argc > 1) ? argv[1] : "AAAAAAAAAAAA";
    struct Packet p = {0};
    printf("copying %zu bytes into buf[8]\n", strlen(payload));
    strcpy(p.buf, payload); // overflow canary/stack
    return 0;
}
