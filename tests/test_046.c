/* old name: structured_memcpy_slices_no_oob.c */
// Copy and shuffle structured slices while respecting bounds (no OOB)
#include <stdio.h>
#include <string.h>

struct Frame
{
    char prefix[12];
    char body[48];
    char suffix[12];
};

static void build_frame(struct Frame *f, const char *a, const char *b)
{
    memset(f, 0, sizeof(*f));

    size_t p = strnlen(a, sizeof(f->prefix) - 1);
    memcpy(f->prefix, a, p);
    f->prefix[p] = '\0';

    size_t b_len = strnlen(b, sizeof(f->body) - 1);
    memcpy(f->body, b, b_len);
    f->body[b_len] = '\0';

    const char *tag = ":ok";
    size_t avail = sizeof(f->body) - b_len - 1;
    strncat(f->body, tag, avail);

    memcpy(f->suffix, "TAIL", 4);
    f->suffix[4] = '\0';
}

static void shuffle_segments(struct Frame *dst, const struct Frame *src)
{
    char scratch[sizeof(dst->body)];
    size_t len = strnlen(src->body, sizeof(src->body));
    memcpy(scratch, src->body, len);
    scratch[len] = '\0';

    // Reverse in place inside scratch, then move back.
    for (size_t i = 0, j = len ? len - 1 : 0; i < j; ++i, --j)
    {
        char tmp = scratch[i];
        scratch[i] = scratch[j];
        scratch[j] = tmp;
    }

    memset(dst, 0, sizeof(*dst));
    strcpy(dst->prefix, src->prefix);
    strcpy(dst->suffix, src->suffix);
    memcpy(dst->body, scratch, len);
    dst->body[len] = '\0';
}

int main(void)
{
    struct Frame frame, shuffled;
    build_frame(&frame, "HEAD", "stack-structured-body");
    shuffle_segments(&shuffled, &frame);

    printf("orig:%s|%s|%s\n", frame.prefix, frame.body, frame.suffix);
    printf("shuf:%s|%s|%s\n", shuffled.prefix, shuffled.body, shuffled.suffix);
    return 0;
}
