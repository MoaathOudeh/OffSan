/* old name: struct_inline_array_tail_overflow.c */
// Overwrite past the tail of a struct's inline array
#include <stdio.h>
#include <string.h>

struct Packet
{
    int opcode;
    char payload[12];
    int trailer;
};

int main(void)
{
    struct Packet pkt;
    memset(&pkt, 0, sizeof(pkt));

    const char *msg = "AAAAAAAAAAAAA";
    printf("copying \"%s\" into payload[12]\n", msg);
    strcpy(pkt.payload, msg); // overflow into trailer/stack

    return 0;
}
