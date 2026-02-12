#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "liboffsan.h"

ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*real_read)(int, void *, size_t) = NULL;
    if (!real_read) {
        real_read = dlsym(RTLD_NEXT, "read");
        if (!real_read) {
            perror("dlsym(read)");
            abort();
        }
    }
    ssize_t ret = real_read(fd, buf, count);

    if (ret > 0) {
        checkStackAccess((uint8_t *)buf, 0, (size_t)ret);
    }

    return ret;
}


char *strcpy(char *dest, const char *src) {
    static char *(*real_strcpy)(char *, const char *) = NULL;
    if (!real_strcpy) {
        real_strcpy = dlsym(RTLD_NEXT, "strcpy");
        if (!real_strcpy) {
            perror("dlsym(strcpy)");
            abort();
        }
    }

    char *res = real_strcpy(dest, src);

    if (res) {
        size_t written = strlen(dest) + 1;
        checkStackAccess((uint8_t *)dest, 0, written);
    }

    return res;
}

char *strncpy(char *dest, const char *src, size_t n) {
    static char *(*real_strncpy)(char *, const char *, size_t) = NULL;
    if (!real_strncpy) {
        real_strncpy = dlsym(RTLD_NEXT, "strncpy");
        if (!real_strncpy) { perror("dlsym(strncpy)"); abort(); }
    }

    char *res = real_strncpy(dest, src, n);

    if (n > 0) {
        checkStackAccess((uint8_t *)dest, 0, n);
    }

    return res;
}

char *strcat(char *dest, const char *src) {
    static char *(*real_strcat)(char *, const char *) = NULL;
    if (!real_strcat) {
        real_strcat = dlsym(RTLD_NEXT, "strcat");
        if (!real_strcat) { perror("dlsym(strcat)"); abort(); }
    }
    size_t dest_len_before = strlen(dest);
    size_t src_len = strlen(src);
    char *res = real_strcat(dest, src);
    checkStackAccess((uint8_t *)dest + dest_len_before, 0, src_len + 1);

    return res;
}

char *strncat(char *dest, const char *src, size_t n) {
    static char *(*real_strncat)(char *, const char *, size_t) = NULL;
    if (!real_strncat) {
        real_strncat = dlsym(RTLD_NEXT, "strncat");
        if (!real_strncat) { perror("dlsym(strncat)"); abort(); }
    }
    size_t dest_len_before = strlen(dest);
    size_t to_copy = strlen(src);
    if (to_copy > n) {
        to_copy = n;
    }

    char *res = real_strncat(dest, src, n);
    checkStackAccess((uint8_t *)dest + dest_len_before, 0, to_copy + 1);

    return res;
}

char *gets(char *s) {
    static char *(*real_gets)(char *) = NULL;
    if (!real_gets) {
        real_gets = dlsym(RTLD_NEXT, "gets");
        if (!real_gets) {
            fprintf(stderr, "[OffSan] gets() not found via dlsym\n");
            abort();
        }
    }

    char *res = real_gets(s);

    if (res) {
        size_t written = strlen(s) + 1;
        checkStackAccess((uint8_t *)s, 0, written);
    }

    return res;
}

char *fgets(char *s, int size, FILE *stream) {
    static char *(*real_fgets)(char *, int, FILE *) = NULL;
    if (!real_fgets) {
        real_fgets = dlsym(RTLD_NEXT, "fgets");
        if (!real_fgets) { perror("dlsym(fgets)"); abort(); }
    }

    char *res = real_fgets(s, size, stream);

    if (res) {
        size_t written = strlen(s) + 1;
        checkStackAccess((uint8_t *)s, 0, written);
    }

    return res;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    static size_t (*real_fread)(void *, size_t, size_t, FILE *) = NULL;
    if (!real_fread) {
        real_fread = dlsym(RTLD_NEXT, "fread");
        if (!real_fread) { perror("dlsym(fread)"); abort(); }
    }

    size_t ret = real_fread(ptr, size, nmemb, stream);

    if (ret > 0 && size > 0) {
        size_t bytes = ret * size;
        checkStackAccess((uint8_t *)ptr, 0, bytes);
    }

    return ret;
}

void *memcpy(void *dest, const void *src, size_t n) {
    //printf("[OffSan:hook] triggered memcpy\n");
    static void *(*real_memcpy)(void *, const void *, size_t) = NULL;
    if (!real_memcpy) {
        real_memcpy = dlsym(RTLD_NEXT, "memcpy");
        if (!real_memcpy) { perror("dlsym(memcpy)"); abort(); }
    }

    void *res = real_memcpy(dest, src, n);

    if (n > 0) {
        checkStackAccess((uint8_t *)dest, 0, n);
    }

    return res;
}

void *memmove(void *dest, const void *src, size_t n) {
    //printf("[OffSan:hook] triggered memmove\n");
    static void *(*real_memmove)(void *, const void *, size_t) = NULL;
    if (!real_memmove) {
        real_memmove = dlsym(RTLD_NEXT, "memmove");
        if (!real_memmove) { perror("dlsym(memmove)"); abort(); }
    }

    void *res = real_memmove(dest, src, n);
    if (n > 0) {
        checkStackAccess((uint8_t *)dest, 0, n);
    }
    return res;
}

void *memset(void *s, int c, size_t n) {
    //printf("[OffSan:hook] triggered memset\n");
    static void *(*real_memset)(void *, int, size_t) = NULL;
    if (!real_memset) {
        real_memset = dlsym(RTLD_NEXT, "memset");
        if (!real_memset) { perror("dlsym(memset)"); abort(); }
    }

    void *res = real_memset(s, c, n);
    if (n > 0) {
        checkStackAccess((uint8_t *)s, 0, n);
    }
    return res;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    static ssize_t (*real_recv)(int, void *, size_t, int) = NULL;
    if (!real_recv) {
        real_recv = dlsym(RTLD_NEXT, "recv");
        if (!real_recv) { perror("dlsym(recv)"); abort(); }
    }

    ssize_t ret = real_recv(sockfd, buf, len, flags);

    if (ret > 0) {
        checkStackAccess((uint8_t *)buf, 0, (size_t)ret);
    }

    return ret;
}

int vsprintf(char *str, const char *format, va_list ap) {
    static int (*real_vsprintf)(char *, const char *, va_list) = NULL;
    if (!real_vsprintf) {
        real_vsprintf = dlsym(RTLD_NEXT, "vsprintf");
        if (!real_vsprintf) { perror("dlsym(vsprintf)"); abort(); }
    }

    va_list ap_copy;
    va_copy(ap_copy, ap);
    int written = real_vsprintf(str, format, ap_copy);
    va_end(ap_copy);

    if (written >= 0) {
        checkStackAccess((uint8_t *)str, 0, (size_t)written + 1);
    }

    return written;
}

int sprintf(char *str, const char *format, ...) {
    static int (*real_vsprintf)(char *, const char *, va_list) = NULL;
    if (!real_vsprintf) {
        real_vsprintf = dlsym(RTLD_NEXT, "vsprintf");
        if (!real_vsprintf) { perror("dlsym(vsprintf)"); abort(); }
    }

    va_list ap;
    va_start(ap, format);
    int written = real_vsprintf(str, format, ap);
    va_end(ap);

    if (written >= 0) {
        checkStackAccess((uint8_t *)str, 0, (size_t)written + 1);
    }

    return written;
}
