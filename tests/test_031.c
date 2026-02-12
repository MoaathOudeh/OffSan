/* old name: pipe_read_overflow_small_buffer.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    int fds[2];
    if (pipe(fds) == 0) {
        const char *msg = "0123456789abcdef";
        write(fds[1], msg, strlen(msg));
        close(fds[1]);

        char small[8];
        read(fds[0], small, 16);
        close(fds[0]);
    }
    return 0;
}
