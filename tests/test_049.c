/* old name: vla_struct_array_no_oob.c */
// VLA of structs with bounded writes (no OOB)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Node
{
    int id;
    char tag[16];
};

int main(int argc, char **argv)
{
    int n = (argc > 1) ? atoi(argv[1]) : 4;
    if (n < 1)
        n = 4;
    if (n > 8)
        n = 8;

    struct Node nodes[n];
    for (int i = 0; i < n; ++i)
    {
        nodes[i].id = i;
        snprintf(nodes[i].tag, sizeof(nodes[i].tag), "node-%d", i);
    }

    printf("vla structs: count=%d tag0=%s\n", n, nodes[0].tag);
    return 0;
}
