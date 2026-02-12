/* old name: foo_bar_inbounds_no_oob.c */
// Safe variant of foo/bar from code.c; writes within bounds
#include <stdio.h>

static void foo(int *p)
{
    for (int i = 0; i < 8; ++i)
        p[i] = i * 2;
}

static void bar(void)
{
    int buf[8];
    foo(buf);
    printf("bar ok: buf[3]=%d\n", buf[3]);
}

int main(void)
{
    bar();
    return 0;
}
