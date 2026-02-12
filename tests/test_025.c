/* old name: non_oob_hook_and_stack_suite.c */
// tests/test_non_oob.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>

struct Pair
{
    int x;
    double y;
};

/* Tests scalar alloca with direct load/store */
static void test_scalar(void)
{
    int scalar = 7;
    scalar += 1;
}

/* Tests VLA alloca (dynamic array size) */
static void test_vla(int n)
{
    char buf[n];
    buf[0] = 'V';
    buf[n - 1] = 'L';
}

/* Tests fixed array alloca plus i8* pointer arithmetic */
static void test_fixed_array(void)
{
    char a[10];
    a[9] = 'a';
    char *p = a + 4;
    p[3] = 'b';
}

/* Tests pointer arithmetic on int* (element size > 1) */
static void test_pointer_arith_on_int(void)
{
    int numbers[6];
    int *ptr = numbers;
    ptr[2] = 42;
    int tmp = ptr[1];
    ptr[1] = tmp + 1;
}

/* Tests struct field access */
static void test_struct_field(void)
{
    struct Pair pair = {1, 2.0};
    double *py = &pair.y;
    *py += 1.0;
}

/* Tests large array alloca */
static void test_large_buffer(void)
{
    char big[512];
    for (int i = 0; i < 512; ++i)
        big[i] = (char)(i & 0xff);

    char *mid = big + 256;
    mid[100] = 'Z';
    char val = mid[0];
    (void)val;
}

/* Hook tests: all should be in-bounds and *not* trigger OOB */

static void test_strcpy_hook_ok(void)
{
    char dst[32];
    strcpy(dst, "OffSanHook");
}

static void test_read_hook_ok(void)
{
    int fds[2];
    if (pipe(fds) != 0)
        return;

    const char *msg = "pipe data";
    write(fds[1], msg, strlen(msg));
    close(fds[1]);

    char buf[32];
    ssize_t n = read(fds[0], buf, sizeof(buf) - 1);
    close(fds[0]);

    if (n > 0)
    {
        buf[n] = '\0';
    }
}

static void test_strncpy_hook_ok(void)
{
    char dst[32];
    strncpy(dst, "OffSan-Strncpy", sizeof(dst));
    dst[sizeof(dst) - 1] = '\0';
}

static void test_strcat_hook_ok(void)
{
    char dst[64] = "OffSan:";
    strcat(dst, "concat");
}

static void test_strncat_hook_ok(void)
{
    char dst[32] = "OffSan:";
    strncat(dst, "xyzuvw", 3);
}

static void test_memcpy_hook_ok(void)
{
    char src[64];
    char dst[64];
    memset(src, 'A', sizeof(src));
    src[sizeof(src) - 1] = '\0';

    memcpy(dst, src, strlen(src) + 1);
}

static void test_memmove_hook_ok(void)
{
    char buf[64] = "0123456789abcdef";
    memmove(buf + 4, buf, 8);
    buf[12] = '\0';
}

static void test_memset_hook_ok(void)
{
    char buf[16];
    memset(buf, 'X', sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
}

static void test_fgets_hook_ok(void)
{
    char buf[32];
    FILE *tmp = tmpfile();
    if (!tmp)
        return;
    fputs("hello fgets", tmp);
    rewind(tmp);
    if (fgets(buf, sizeof(buf), tmp))
    {
    }
    fclose(tmp);
}

static void test_fread_hook_ok(void)
{
    char buf[32];
    FILE *tmp = tmpfile();
    if (!tmp)
        return;
    const char data[] = "hello fread";
    fwrite(data, 1, sizeof(data), tmp);
    rewind(tmp);
    size_t n = fread(buf, 1, sizeof(data), tmp);
    if (n > 0)
    {
        if (n >= sizeof(buf))
            n = sizeof(buf) - 1;
        buf[n] = '\0';
    }
    fclose(tmp);
}

static void test_recv_hook_ok(void)
{
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0)
        return;
    const char msg[] = "socket-data";
    write(sv[1], msg, strlen(msg));
    char buf[32];
    ssize_t n = recv(sv[0], buf, sizeof(buf), 0);
    if (n > 0)
    {
        if (n >= (ssize_t)sizeof(buf))
            n = (ssize_t)sizeof(buf) - 1;
        buf[n] = '\0';
    }
    close(sv[0]);
    close(sv[1]);
}

/* New main that runs all non-OOB tests */

int main(void)
{
    int i = 0;
    printf("[+] test %d\n", i);
    i++;
    test_scalar();

    printf("[+] test %d\n", i);
    i++;
    test_vla(12);

    printf("[+] test %d\n", i);
    i++;
    test_fixed_array();

    printf("[+] test %d\n", i);
    i++;
    test_pointer_arith_on_int();

    printf("[+] test %d\n", i);
    i++;
    test_struct_field();

    printf("[+] test %d\n", i);
    i++;
    test_large_buffer();

    printf("[+] test %d\n", i);
    i++;
    test_strcpy_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_read_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_strncpy_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_strcat_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_strncat_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_memcpy_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_memmove_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_memset_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_fgets_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_fread_hook_ok();

    printf("[+] test %d\n", i);
    i++;
    test_recv_hook_ok();

    return 0;
}
