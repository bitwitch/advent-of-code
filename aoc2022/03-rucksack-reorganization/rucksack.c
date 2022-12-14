/* after generating a gif, i used gifsicle to optimize:
 * gifsicle -d6 rucksack.gif -O3 -o out.gif --colors 16
 *
 * size went from 1.7MB to 115KB on my machine
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

typedef struct {
    int indices[32];
    int count;
} Matches;

void emphasize_matches(Matches matches, int x_off, int y, int scale, char item, uint32_t color) {
    for (int i=0; i<matches.count; ++i) {
        drawstringf(18*matches.indices[i] + x_off, y, scale, color, "%c", item);
    }
}

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

    int i;
    uint32_t color_one, color_two;

    int chars_per_frame = 2;
    int c = 0;

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
            emphasize_matches(matches_one, 10, y, scale, item, 0x00ff00);
            emphasize_matches(matches_two, comp_offset+10, y, scale, item, 0x00ff00);
            nextframe();

            emphasize_matches(matches_one, 10, y, scale, item, 0x00ffff);
            emphasize_matches(matches_two, comp_offset+10, y, scale, item, 0x00ffff);
            nextframe();

            emphasize_matches(matches_one, 10, y, scale, item, 0xffffff);
            emphasize_matches(matches_two, comp_offset+10, y, scale, item, 0xffffff);
            nextframe();

            emphasize_matches(matches_one, 10, y, scale, item, 0x00ffff);
            emphasize_matches(matches_two, comp_offset+10, y, scale, item, 0x00ffff);
            nextframe();

            emphasize_matches(matches_one, 10, y, scale, item, 0x00ff00);
            emphasize_matches(matches_two, comp_offset+10, y, scale, item, 0x00ff00);
            nextframe();


            break;
        }

        ++c;
        if (c % chars_per_frame == 0)
            nextframe();
    }
    y += 25;
}


void bit_mask_row(uint64_t mask, int x, int y, int side, uint32_t color_off, uint32_t color_on) {
    int left;
    uint32_t color;

    for (int i=0; i<52; ++i) {
        left = x + i*side + 1;
        color = (mask >> i) & 1 ? color_on : color_off;
        drawrect(left, y, left+side, y+side, color);
    }
}

int get_priority(char c) {
    /* Lowercase item types a through z have priorities 1 through 26.
     * Uppercase item types A through Z have priorities 27 through 52. */
    return (c >= 97 && c <= 122) ? c - 96 : c - 38;
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
    int bit_index, common_bit = 0;
    static int y = 0;

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
        if (((seen0 & seen1 & seen2) >> bit_index) & 1) {
            common_bit = bit_index;
        }
    }

    // draw bit masks as row of rectangles
    int side = 18;
    int margin_left = 10;
    int x_emphasize = margin_left + common_bit*side + 1;
    int y0, y1, y2;

    bit_mask_row(seen0, margin_left, y, side, 0x000000, 0xFFFFFF);
    y0 = y;
    y += side;
    nextframe();

    bit_mask_row(seen1, margin_left, y, side, 0x000000, 0xFFFFFF);
    y1 = y;
    y += side;
    nextframe();

    bit_mask_row(seen2, margin_left, y, side, 0x000000, 0xFFFFFF);
    y2 = y;
    y += side;
    nextframe();

    // emphasize common bit in each row
    drawrect(x_emphasize, y0, x_emphasize+side, y0+side, 0x00FF00);
    drawrect(x_emphasize, y1, x_emphasize+side, y1+side, 0x00FF00);
    drawrect(x_emphasize, y2, x_emphasize+side, y2+side, 0x00FF00);
    nextframe();
    drawrect(x_emphasize, y0, x_emphasize+side, y0+side, 0x00FFFF);
    drawrect(x_emphasize, y1, x_emphasize+side, y1+side, 0x00FFFF);
    drawrect(x_emphasize, y2, x_emphasize+side, y2+side, 0x00FFFF);
    nextframe();
    drawrect(x_emphasize, y0, x_emphasize+side, y0+side, 0x00FF00);
    drawrect(x_emphasize, y1, x_emphasize+side, y1+side, 0x00FF00);
    drawrect(x_emphasize, y2, x_emphasize+side, y2+side, 0x00FF00);
    nextframe();

    return common_bit + 1;
}

void part_one(uint8_t *input) {
    clear();
    uint64_t sum = 0;
    char *line;

    do {
        line = arb_chop_by_delimiter((char**)&input, "\n");
        char duplicate = find_duplicate(line);
        visualize_part_one(line, duplicate);
        sum += get_priority(duplicate);
    } while (*input != '\0');

    printf("part_one: %lu\n", sum);
}

void part_two(uint8_t *input) {
    clear();
    uint64_t sum = 0;
    char *line0, *line1, *line2;

    do {
        line0 = arb_chop_by_delimiter((char**)&input, "\n");
        line1 = arb_chop_by_delimiter((char**)&input, "\n");
        line2 = arb_chop_by_delimiter((char**)&input, "\n");
        sum += group_badge_priority(line0, line1, line2);
    } while (*input != '\0');

    printf("part_two: %lu\n", sum);
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


    setupgif(0, 2, "rucksack.gif");

    part_one(file_copy);
    part_two(file_data);

    endgif();

    return 0;
}
