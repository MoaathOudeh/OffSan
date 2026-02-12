/* old name: pipeline_transformations_no_oob.c */
// Multi-stage data shaping with guarded stack writes (in-bounds)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Stage
{
    char header[16];
    char payload[64];
    char trailer[8];
};

static size_t copy_clamped(char *dst, size_t cap, const char *src)
{
    size_t n = strnlen(src, cap - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
    return n;
}

static void rotate_payload(struct Stage *s, size_t step)
{
    size_t len = strnlen(s->payload, sizeof(s->payload));
    if (len == 0)
        return;

    char tmp[64];
    memcpy(tmp, s->payload, len);
    tmp[len] = '\0';

    step %= len;
    for (size_t i = 0; i < len; ++i)
        s->payload[i] = tmp[(i + step) % len];
    s->payload[len] = '\0';
}

static void append_checksum(struct Stage *s)
{
    unsigned sum = 0;
    size_t len = strnlen(s->payload, sizeof(s->payload));
    for (size_t i = 0; i < len; ++i)
        sum += (unsigned char)s->payload[i];

    char tail[8];
    snprintf(tail, sizeof(tail), "%02x", sum & 0xff);

    size_t avail = sizeof(s->payload) - len - 1;
    strncat(s->payload, tail, avail);
}

int main(int argc, char **argv)
{
    const char *input = (argc > 1) ? argv[1] : "pipeline-safe-data";
    size_t rotate = (argc > 2) ? (size_t)atoi(argv[2]) : 5;

    struct Stage stage = {0};
    strcpy(stage.header, "BEGIN");
    strcpy(stage.trailer, "END");
    copy_clamped(stage.payload, sizeof(stage.payload), input);

    rotate_payload(&stage, rotate);
    append_checksum(&stage);

    printf("header=%s payload=%s trailer=%s\n", stage.header, stage.payload, stage.trailer);
    return 0;
}
