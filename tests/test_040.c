/* old name: strcat_runs_past_stack_buffer.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    char base[8] = "abc";
    strcat(base, "defghi");
    return 0;
}
