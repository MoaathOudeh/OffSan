/* old name: stack_use_after_return_overflow.c */
// Stack use-after-scope: captures a pointer to a dead frame and mutates it
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *make_dangling(void)
{
    char local[24];
    strcpy(local, "dangling-stack-pointer");
    return local;
}

int main(void)
{
    char *ptr = make_dangling();
    printf("%s", ptr);
    printf("dangling ptr=%p\n", (void *)ptr);
    strcpy(ptr, "write-after-return-oob-trigger");
    return 0;
}
