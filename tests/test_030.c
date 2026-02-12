/* old name: packet_struct_memcpy_no_oob.c */
// Struct copy derived from code.c branches; all accesses are in-bounds
#include <stdio.h>
#include <string.h>

struct Packet
{
    char buf[8];
    int canary;
};

int main(void)
{
    struct Packet a = {.buf = "HELLO", .canary = 0x1234};
    struct Packet b = {0};
    memcpy(&b, &a, sizeof(b));
    printf("packet ok: canary=0x%x buf=%s\n", b.canary, b.buf);
    return 0;
}
