/* old name: socket_recv_into_small_buffer_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
        char buf[8];
        write(sv[1], "0123456789abcdef", 16);
        recv(sv[0], buf, 16, 0);
        close(sv[0]);
        close(sv[1]);
    }
    return 0;
}
