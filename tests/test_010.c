/* old name: foo_bar_helper_overwrite_overflow.c */
// Overflow variant of foo/bar: helper writes past caller's buffer
#include <stdio.h>

static void foo(int *p)
{
    for (int i = 0; i < 16; ++i) // buf has only 8 ints
        p[i] = 42 + i;
}

static void bar(void)
{
    int buf[8];
    foo(buf); // overflow into caller frame
}

int main(void)
{
    bar();
    puts("should not reach here cleanly");
    return 0;
}
