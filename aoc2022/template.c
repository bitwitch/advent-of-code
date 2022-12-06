#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

void part_one(uint8_t *input) {
    printf("part_one: %d\n", 696969);
}

void part_two(uint8_t *input) {
    printf("part_two: %d\n", 696969);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

    FILE *fp;
    uint8_t *file_data, *file_copy;
    size_t file_size;

    fp = fopen(argv[1], "r");
    if (!fp) { perror("fopen"); exit(1); }

    int ok = arb_read_entire_file(fp, &file_data, &file_size);
    if (ok != ARB_READ_ENTIRE_FILE_OK) {
        fprintf(stderr, "Error: arb_read_entire_file()\n");
        fclose(fp);
        exit(1);
    }

    fclose(fp);

    file_copy = malloc(file_size);
    if (!file_copy) { perror("malloc"); exit(1); }
    memcpy(file_copy, file_data, file_size);

    part_one(file_copy);
    part_two(file_data);

    return 0;
}
