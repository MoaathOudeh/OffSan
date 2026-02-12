/* old name: loop_off_by_one_overflow.c */
// Off-by-one write past stack buffer end
#include <stdio.h>

int main(void)
{
    char data[16] = {0};
    for (int i = 0; i <= 16; ++i) // i == 16 is out of bounds
        data[i] = 'X';

    puts("finished loop");
    return 0;
}
