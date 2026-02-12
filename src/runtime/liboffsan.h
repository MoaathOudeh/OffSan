#include <stdint.h>

void initStackVar(uint8_t *addr, int64_t size);
void checkStackAccess(uint8_t *baseAddr, int64_t offset, int64_t accessSize);
void offsan_frame_enter();
void offsan_frame_exit();