/* old name: chunked_socket_pipeline_overflow.c */
// Chunked recv loop that walks past a fixed stack buffer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void send_chunks(int fd, const char *payload)
{
    size_t len = strlen(payload);
    size_t sent = 0;
    while (sent < len)
    {
        size_t chunk = (len - sent > 16) ? 16 : (len - sent);
        send(fd, payload + sent, chunk, 0);
        sent += chunk;
    }
    shutdown(fd, SHUT_WR);
}

static void recv_unchecked(int fd)
{
    char buf[24] = {0};
    size_t offset = 0;
    ssize_t got;
    while ((got = recv(fd, buf + offset, 16, 0)) > 0)
        offset += (size_t)got; // offset can exceed buffer size

    printf("received=%zu bytes\n", offset);
}

int main(void)
{
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0)
        return 1;

    const char *payload = "chunked-stream-that-is-much-longer-than-the-buffer";
    send_chunks(sv[0], payload);
    recv_unchecked(sv[1]);

    close(sv[0]);
    close(sv[1]);
    return 0;
}
