/* old name: overflow_smashes_stack_function_pointer.c */
// Overflow clobbers a function pointer on the stack
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void benign(void)
{
    puts("benign");
}

static void injected(void)
{
    puts("injected");
}

int main(int argc, char **argv)
{
    void (*fn)(void) = benign;
    char buf[16];
    const char *payload = (argc > 1) ? argv[1] : "AAAAAAAAAAAAAAAAAAAA";

    printf("copying %zu bytes into buf[16]\n", strlen(payload));
    strcpy(buf, payload); // overflow likely, may smash fn

    if (fn == benign)
        fn();
    else
        injected();

    return 0;
}
