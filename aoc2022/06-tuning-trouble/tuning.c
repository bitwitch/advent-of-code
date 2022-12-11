#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

/* NOTE(shaw) this is not trying to be smart at all */
bool all_characters_unique(uint8_t *str, size_t size) { 
    int i;
    for (i=0; i<(int)size-1; ++i)
        if (memchr(str+i+1, str[i], size-i-1))
            return false;
    return true;
}

U32 characters_processed_before_marker_detected(uint8_t *input, size_t input_size, size_t marker_length) {
    size_t i;
    U32 result = 0;

    for (i=0; i < input_size - marker_length; ++i) {

        // draw window we are currently checking
        //
        //

        if (all_characters_unique(input+i, marker_length)) {
            result = i + marker_length;

            // animation showing this check was unique
            //

            break;
        }
    }
    return result;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

    FILE *fp;
    uint8_t *file_data;
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

    U32 part_one = characters_processed_before_marker_detected(file_data, file_size, 4);
    U32 part_two = characters_processed_before_marker_detected(file_data, file_size, 14);

    printf("part one: %d\npart two: %d\n", part_one, part_two);

    return 0;
}
