/* old name: recursive_vla_matrix_overflow.c */
// Recursive VLA matrix filling with deliberately loose bounds (overflow)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fill_layer(int rows, int cols, int depth)
{
    int grid[rows][cols];
    memset(grid, 0, sizeof(grid));

    for (int r = 0; r <= rows; ++r)
    {
        for (int c = 0; c < cols + depth; ++c)
            grid[r][c] = (r + 1) * (c + 1); // r==rows or c>=cols -> overflow on purpose
    }

    if (depth > 0)
        return fill_layer(rows, cols + 1, depth - 1) + grid[0][0];
    return grid[0][0];
}

int main(int argc, char **argv)
{
    int rows = (argc > 1) ? atoi(argv[1]) : 4;
    int cols = (argc > 2) ? atoi(argv[2]) : 4;
    int depth = (argc > 3) ? atoi(argv[3]) : 2;

    if (rows < 2)
        rows = 2;
    if (cols < 2)
        cols = 2;

    printf("rows=%d cols=%d depth=%d\n", rows, cols, depth);
    int result = fill_layer(rows, cols, depth);
    printf("result=%d\n", result);
    return result;
}
