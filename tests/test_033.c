/* old name: pointer_delta_computed_oob_write.c */
// Compute delta between two stack arrays then write at that offset (can go OOB)
#include <stdio.h>
#include <string.h>

int main(void)
{
    char a[8] = {0};
    char b[8] = {0};

    char *first = a;
    char *second = b;
    if (&a > &b)
    {
        first = b;
        second = a;
    }

    int delta = (int)(second - first);
    printf("delta=%d (writing first[delta])\n", delta);
    first[delta] = 'X'; // likely OOB
    return 0;
}
