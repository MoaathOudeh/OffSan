/* old name: argv_concat_overflow.c */
// Intentional overflow through repeated concatenation of argv chunks
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buf[16] = "";

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
            strcat(buf, argv[i]); // overflow likely with long argv
    }
    else
    {
        const char *chunks[] = {"chunk", "-0123456789", "-overflow"};
        for (size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i)
            strcat(buf, chunks[i]); // guaranteed overflow on the final append
    }

    printf("concat result: %s\n", buf);
    return 0;
}
