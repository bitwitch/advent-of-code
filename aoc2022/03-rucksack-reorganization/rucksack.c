#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

int get_priority(char c) {
    /* Lowercase item types a through z have priorities 1 through 26.
     * Uppercase item types A through Z have priorities 27 through 52. */
    return (c >= 97 && c <= 122) ? c - 96 : c - 38;
}

typedef struct {
    int indices[32];
    int count;
} Matches;

void visualize_part_one(char *rucksack, char item) {
    static int y = 10, scale = 2;
    int x = 10;

    if (y > 1024) return;

    Matches matches_one = {0};
    Matches matches_two = {0};

    bool found_one = false;
    bool found_two = false;
    int comp_size  = strlen(rucksack)/2;
    char *comp_one = rucksack;
    char *comp_two = comp_one + comp_size;
    /*int comp_offset = 18 * (comp_size+1);*/
    int comp_offset = 18 * 23;
    int i, j;
    uint32_t color, color_one, color_two;

    int chars_per_frame = 2;
    int c;

    for (i=0; i<comp_size; ++i) {

        if (x > 1024) continue;

        color_one = color_two = 0xFFFFFF;
        if (comp_one[i] == item) {
            matches_one.indices[matches_one.count++] = i;
            if (found_two) color_one = 0x00FF00;
            found_one = true;
        }
        if (comp_two[i] == item) {
            matches_two.indices[matches_two.count++] = i;
            if (found_one) color_two = 0x00FF00;
            found_two = true;
        }
 
        drawstringf(x, y, scale, color_one, "%c", comp_one[i]); 
        drawstringf(x+comp_offset, y, scale, color_two, "%c", comp_two[i]); 
        x += 18;

        if (found_one && found_two) {
            for (j=0; j<matches_one.count; ++j)
                drawstringf(18*matches_one.indices[j] + 10, y, scale, 0x00ff00, "%c", item); 
            for (j=0; j<matches_two.count; ++j)
                drawstringf(18*matches_two.indices[j] + comp_offset + 10, y, scale, 0x00ff00, "%c", item); 
            nextframe(0);
            break;
        }

        ++c;
        if (c % chars_per_frame == 0)
            nextframe(0);
    }
    y += 25;
}

char find_duplicate(char *rucksack) {
    uint64_t seen_one = 0;
    uint64_t seen_two = 0;

    int comp_size  = strlen(rucksack)/2;
    char *comp_one = rucksack;
    char *comp_two = comp_one + comp_size;

    int i, bit_index;

    for (i=0; i<comp_size; ++i) {
        // next item in compartment one
        bit_index = get_priority(*comp_one) - 1;
        if (seen_two & ((uint64_t)1 << bit_index)) {
            return *comp_one;
        }
        seen_one |= (uint64_t)1 << bit_index;
        ++comp_one;

        // next item in compartment two
        bit_index = get_priority(*comp_two) - 1;
        if (seen_one & ((uint64_t)1 << bit_index)) {
            return *comp_two;
        }
        seen_two |= (uint64_t)1 << bit_index;
        ++comp_two;
    }

    assert(0 && "rucksack supplied to find_duplicate must contain a single duplicate");
    return '\0';
}

int group_badge_priority(char *line0, char *line1, char *line2) {
    uint64_t seen0 = 0, 
             seen1 = 0, 
             seen2 = 0;

    int i;
    int bit_index;

    /* store bitmask of items in each rucksack */
    for (i=0; i<strlen(line0); ++i) {
        bit_index = get_priority(line0[i]) - 1;
        seen0 |= (uint64_t)1 << bit_index;
    }
    for (i=0; i<strlen(line1); ++i) {
        bit_index = get_priority(line1[i]) - 1;
        seen1 |= (uint64_t)1 << bit_index;
    }
    for (i=0; i<strlen(line2); ++i) {
        bit_index = get_priority(line2[i]) - 1;
        seen2 |= (uint64_t)1 << bit_index;
    }

    /* find the one item all three rucksacks have in common */
    for (bit_index = 0; bit_index < 64; ++bit_index) {
        if (((seen0 & seen1 & seen2) >> bit_index) & 1)
            return bit_index + 1;
    }

    assert(0 && "the three rucksacks must all have one single item in common");
    return 0;
}

void part_one(uint8_t *input) {
    setupgif(0, 2, "rucksack.gif");
    clear();

    uint64_t sum = 0;
    char *line;
    int i;

    do {
        line = arb_chop_by_delimiter((char**)&input, "\n");
        char duplicate = find_duplicate(line);
        visualize_part_one(line, duplicate);
        sum += get_priority(duplicate);
    } while (*input != '\0');

    printf("part_one: %d\n", sum);


    endgif();
}

void part_two(uint8_t *input) {
    uint8_t *remaining = input;
    uint64_t sum = 0;
    char *line0, *line1, *line2;
    int i;

    do {
        line0 = arb_chop_by_delimiter((char**)&input, "\n");
        line1 = arb_chop_by_delimiter((char**)&input, "\n");
        line2 = arb_chop_by_delimiter((char**)&input, "\n");
        sum += group_badge_priority(line0, line1, line2);
    } while (*input != '\0');

    printf("part_two: %d\n", sum);
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
