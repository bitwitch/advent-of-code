#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"


void part_one(uint8_t *input) {
    /*
     * Lowercase item types a through z have priorities 1 through 26.
     * Uppercase item types A through Z have priorities 27 through 52.
     */
     /*uint64_t seen = 0;*/

    char *line;
    int i;

    for (i = 0; *line != '\0'; ++i) {
        line = arb_chop_by_delimiter((char**)&input, "\n");
        if (*line != '\0')
            printf("line %d: %s\n", i, line);
    } 


    printf("part_one: %d\n", 69);

}

void part_two(uint8_t *input) {
    printf("part_two: %d\n", 69);
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

    part_one(file_data);
    part_two(file_data);

    return 0;
}
