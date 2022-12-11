#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

bool all_characters_unique(uint8_t *str, size_t size) { 
    int i;
    for (i=0; i<(int)size-1; ++i)
        if (memchr(str+i+1, str[i], size-i-1))
            return false;
    return true;
}

void part_one(uint8_t *input, size_t input_size) {
    size_t window_size = 4;
    size_t i;
    U32 result = 0;

    for (i=0; i < input_size - window_size; ++i) {
        if (all_characters_unique(input+i, window_size)) {
            result = i + window_size;
            break;
        }
    }

    printf("part_one: %d\n", result);
}

void part_two(uint8_t *input, size_t input_size) {
    int window_size = 14;
    int i;
    U32 result = 0;

    for (i=0; i < (int)input_size - window_size; ++i) {
        if (all_characters_unique(input+i, window_size)) {
            result = i + window_size;
            break;
        }
    }

    printf("part_two: %d\n", result);
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

    part_one(file_copy, file_size);
    part_two(file_data, file_size);

    return 0;
}
