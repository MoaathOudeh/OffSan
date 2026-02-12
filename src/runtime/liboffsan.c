#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>
#include <limits.h>
#include "liboffsan.h"


#define OFFSAN_MAX_DEPTH 1024
typedef struct {
    uint8_t *real_base;     // stack base
    size_t   stack_size;    // stack size
    uint8_t *shadow_base;   // mmap'd shadow
} offsan_tls_t;

static offsan_tls_t g_tls = {0};

static  size_t saved_min[OFFSAN_MAX_DEPTH];
static  size_t saved_max[OFFSAN_MAX_DEPTH];
static  unsigned depth = 0;

static  size_t cur_min = (size_t)-1;
static  size_t cur_max = 0;

static int offsan_debug_enabled(void) {
    static int init = 0;
    static int val = 0;
    if (!init) {
        const char *e = getenv("OFFSAN_DEBUG");
        val = (e && *e && strcmp(e, "0") != 0) ? 1 : 0;
        init = 1;
    }
    return val;
}

static int offsan_continue_enabled(void){
    static int init = 0;
    static int val = 0;
    if (!init) {
        const char *e = getenv("OFFSAN_CONTINUE");
        val = (e && *e && strcmp(e, "0") != 0) ? 1 : 0;
        init = 1;
    }
    return val;
}

#define DLOG(...)  do { if (offsan_debug_enabled()) fprintf(stderr, __VA_ARGS__); } while (0)


static void get_real_stack_bounds(uint8_t **base_out, size_t *size_out) {
    pthread_attr_t attr;
    if (pthread_getattr_np(pthread_self(), &attr) != 0) {
        perror("pthread_getattr_np");
        abort();
    }
    void *b = NULL; size_t sz = 0;
    if (pthread_attr_getstack(&attr, &b, &sz) != 0) {
        pthread_attr_destroy(&attr);
        perror("pthread_attr_getstack");
        abort();
    }
    pthread_attr_destroy(&attr);
    *base_out = (uint8_t *)b;
    *size_out = sz;
}

static void ensure_initialized(void) {
    if (g_tls.shadow_base)
        return;

    get_real_stack_bounds(&g_tls.real_base, &g_tls.stack_size);

    g_tls.shadow_base = mmap(NULL, g_tls.stack_size,
                             PROT_READ | PROT_WRITE,
                             MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (g_tls.shadow_base == MAP_FAILED) {
        perror("mmap shadow");
        abort();
    }
    DLOG("[OffSan] real base=%p size=0x%zx\n"
         "[OffSan] shadow base=%p size=0x%zx\n",
         g_tls.real_base, g_tls.stack_size,
         g_tls.shadow_base, g_tls.stack_size);
}

/* =========================================================
   Shadow layout:
   For an object of N bytes starting at addr A, we lay down:
   shadow[A+0] = min(N-1, 0xFF)
   shadow[A+1] = min(N-2, 0xFF)
   ...
   shadow[A+i] = min(N-1-i, 0xFF)
   ========================================================= */
static inline void fill_shadow_bytes(uint8_t *dst, size_t bytes) {
    if (bytes == 0) return;

    size_t remaining = bytes;
    for (size_t i = 0; i < bytes; ++i) {
        size_t v = remaining ? (remaining - 1) : 0;
        dst[i] = (uint8_t)(v > 0xff ? 0xff : v);
        if (remaining) --remaining;
    }
}

void offsan_frame_enter(void) {
    ensure_initialized();
    if (depth < OFFSAN_MAX_DEPTH) {
        saved_min[depth] = cur_min;
        saved_max[depth] = cur_max;
        depth++;
    }
    cur_min = (size_t)-1;
    cur_max = 0;
}

void offsan_frame_exit(void) {
    ensure_initialized();
    if (cur_min != (size_t)-1 && cur_max > cur_min) {
        memset(g_tls.shadow_base + cur_min, 0, cur_max - cur_min);
    }
    if (depth > 0) {
        depth--;
        cur_min = saved_min[depth];
        cur_max = saved_max[depth];
    } else {
        cur_min = (size_t)-1;
        cur_max = 0;
    }
}

void initStackVar(uint8_t *addr, int64_t size) {
    ensure_initialized();

    if (size <= 0 || addr == NULL) return;

    intptr_t off = (intptr_t)(addr - g_tls.real_base);
    if (off < 0 || (size_t)off >= g_tls.stack_size) {
        // Not on the stack (e.g., heap/global) → ignore
        return;
    }
    
    size_t n = (size_t)size;
    DLOG("[OffSan:init] addr=%p size=%ld off=0x%zx\n", addr, (long)size, (size_t)off);
    fill_shadow_bytes(g_tls.shadow_base + off, n);

    size_t o = (size_t)off;
    size_t end = o + n;
    if (o < cur_min) cur_min = o;
    if (end > cur_max) cur_max = end;

}

/* Compute how many bytes can be written starting at 'baseOff' in the
   *current* object, using the decreasing shadow scheme.
   -> Fast path: if the first marker < 0xff, that's the full answer.
   -> Slow path: if it is 0xff, jump in 255-byte strides and accumulate until tail.
*/
static inline int64_t compute_allowed_max_index(size_t baseOff) {
    if (baseOff >= g_tls.stack_size) return -1;

    uint8_t first = g_tls.shadow_base[baseOff];
    if (first < 0xff) {
        return (int64_t)first; // 0..254 ⇒ size = first+1 bytes remaining
    }
    // ≥255 remaining: accumulate full 255 chunks plus tail
    int64_t total = first;
    size_t cursor = baseOff + 0xff;
    while (cursor < g_tls.stack_size) {
        uint8_t m = g_tls.shadow_base[cursor];
        total += m;
        if (m < 0xff) break;
        cursor += 0xff;
    }
    return total;
}

static inline int is_stack_ptr(const void *p) {
    ensure_initialized();
    if (!p) return 0;
    intptr_t off = (const uint8_t *)p - g_tls.real_base;
    return (off >= 0 && (size_t)off < g_tls.stack_size);
}

void checkStackAccess(uint8_t *baseAddr, int64_t offset, int64_t accessSize) {
    if (accessSize <= 0) return;
    if (!is_stack_ptr(baseAddr)) return;

    // Reject negative offsets outright
    if (offset < 0) {
        fprintf(stderr, "[OffSan:OOB] (negative offset) base=%p off=%ld size=%ld\n",
                baseAddr, (long)offset, (long)accessSize);
        if(!offsan_continue_enabled()){
            abort();
        }
    }

    size_t baseOff = (size_t)(baseAddr - g_tls.real_base);
    int64_t allowed_max = compute_allowed_max_index(baseOff);
    int64_t need_max = offset + accessSize - 1;

    if (allowed_max == 0 && compute_allowed_max_index(baseOff-1) == 0){
        // Not tracked by initStackVar so we skip it.
        // This might be caused by using stack variables from code that
        // was not compiled by offsan
        return;
    }

    DLOG("[OffSan:chk] addr=%p off=%ld need=%ld allowed=%ld\n",
         baseAddr, (long)offset, (long)need_max, (long)allowed_max);

    if (allowed_max < 0 || need_max > allowed_max) {
        fprintf(stderr,
                "[OffSan:OOB] base=%p need_max=%ld allowed_max=%ld (off=%ld, size=%ld)\n",
                baseAddr, (long)need_max, (long)allowed_max,
                (long)offset, (long)accessSize);
        if(!offsan_continue_enabled()){
            abort();
        }
    }
}


