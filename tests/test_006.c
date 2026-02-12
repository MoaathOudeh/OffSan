/* old name: basic_pointer_no_oob.c */
#include <stdlib.h>
#include <stdio.h>

void foo(int *p) {
    p[3] = 42;
}

void bar(void) {
    int buf[8];
    foo(buf);
}

int main()
{   
    bar();
    return 0;
}