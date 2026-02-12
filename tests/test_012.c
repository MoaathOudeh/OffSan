/* old name: fread_beyond_buffer_capacity_oob.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>


int main(void) {
    FILE *tmp = tmpfile();
    if (tmp) {
        char buf[8];
        fwrite("abcdefghijk", 1, 11, tmp);
        rewind(tmp);
        fread(buf, 1, 11, tmp);
        fclose(tmp);
    }
    return 0;
}
