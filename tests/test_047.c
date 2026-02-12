/* old name: switch_selected_buffers_no_oob.c */
// Multiple stack buffers chosen via switch (no OOB)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int mode = (argc > 1) ? atoi(argv[1]) : 1;
    const char *msg = "payload";

    char tiny[8];
    char small[16];
    char medium[32];

    switch (mode)
    {
    case 0:
        strncpy(tiny, msg, sizeof(tiny) - 1);
        tiny[sizeof(tiny) - 1] = '\0';
        puts(tiny);
        break;
    case 1:
        strncpy(small, msg, sizeof(small) - 1);
        small[sizeof(small) - 1] = '\0';
        puts(small);
        break;
    default:
        strncpy(medium, msg, sizeof(medium) - 1);
        medium[sizeof(medium) - 1] = '\0';
        puts(medium);
        break;
    }

    return 0;
}
