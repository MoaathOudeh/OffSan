/* old name: basic_stack_ops_no_oob.c */
// Basic no-OOB sanity checks: fixed arrays, VLAs, simple arithmetic
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int sum_array(const int *arr, size_t n)
{
    int total = 0;
    for (size_t i = 0; i < n; ++i)
        total += arr[i];
    return total;
}

int main(void)
{
    char greeting[32] = "hello";
    strcat(greeting, "-offsan");

    int numbers[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int total = sum_array(numbers, 8);

    size_t dyn = 12;
    char vla[dyn];
    memset(vla, 0xAB, dyn);
    vla[dyn - 1] = '\0';

    uint8_t buffer[64];
    for (size_t i = 0; i < sizeof(buffer); ++i)
        buffer[i] = (uint8_t)(i & 0xff);

    printf("greeting=%s total=%d tail=%d\n", greeting, total, buffer[63]);
    return 0;
}
