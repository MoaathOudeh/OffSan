/* old name: hooked_string_and_io_no_oob.c */
// Hook coverage without triggering OOB
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

static void run_string_ops(void)
{
    char dst[32];
    strcpy(dst, "base");
    strncat(dst, "-xyz", 4);

    char tmp[16];
    memset(tmp, 0, sizeof(tmp));
    memcpy(tmp, dst, 8);
    memmove(tmp + 4, tmp, 8);

    char fmt[24];
    snprintf(fmt, sizeof(fmt), "%s-%d", "ok", 42);
    (void)fmt;
}

static void run_io_ops(void)
{
    int fds[2];
    if (pipe(fds) != 0)
        return;

    const char msg[] = "hook-data";
    write(fds[1], msg, sizeof(msg));
    close(fds[1]);

    char buf[32];
    ssize_t n = read(fds[0], buf, sizeof(buf));
    if (n > 0 && n < (ssize_t)sizeof(buf))
        buf[n] = '\0';
    close(fds[0]);
}

static void run_socket_ops(void)
{
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0)
        return;

    const char msg[] = "socket-payload";
    send(sv[0], msg, sizeof(msg), 0);

    char buf[32];
    recv(sv[1], buf, sizeof(buf), 0);
    close(sv[0]);
    close(sv[1]);
}

int main(void)
{
    run_string_ops();
    run_io_ops();
    run_socket_ops();
    puts("hook ok");
    return 0;
}
