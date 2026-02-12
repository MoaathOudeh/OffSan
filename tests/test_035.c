/* old name: recursive_vla_frames_no_oob.c */
// Safe recursive frames with VLAs to exercise instrumentation on call chains
#include <stdio.h>

static int recursive_fill(int depth)
{
    int local[16];
    for (int i = 0; i < 16; ++i)
        local[i] = depth + i;

    int accum = local[depth % 16];
    if (depth == 0)
        return accum;
    return accum + recursive_fill(depth - 1);
}

int main(void)
{
    int result = recursive_fill(8);
    printf("recursive result=%d\n", result);
    return 0;
}
