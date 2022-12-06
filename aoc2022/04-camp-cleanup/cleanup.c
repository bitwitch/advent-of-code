#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../base/base_inc.h"
#include "../base/base_inc.c"

typedef struct {
    uint32_t lo, hi;
} Interval;

void part_one(Interval *intervals, int intervals_count) {
    int count = 0;
    int i;
    Interval a, b;

    for (i=0; i<intervals_count-1; i+=2) {
        a = intervals[i];
        b = intervals[i+1];
        if ((a.lo >= b.lo && a.hi <= b.hi) || (b.lo >= a.lo && b.hi <= a.hi))
            ++count;
    } 

    printf("part_one: %d\n", count);
}

void part_two(Interval *intervals, int intervals_count) {
    int count = 0;
    int i;
    Interval a, b;

    for (i=0; i<intervals_count-1; i+=2) {
        a = intervals[i];
        b = intervals[i+1];

        if ((a.lo >= b.lo && a.lo <= b.hi) ||
            (a.hi >= b.lo && a.hi <= b.hi) ||
            (b.lo >= a.lo && b.lo <= a.hi) ||
            (b.hi >= a.lo && b.hi <= a.hi))
        {
            ++count;
        }
    }

    printf("part_two: %d\n", count);
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

    char *line;
    size_t used = 0, size = 0; // in bytes
    Interval *intervals = NULL, *temp;
    int n = 0; // intervals count

    do {
        used += 2 * sizeof(Interval);
        
        // grow storage when needed
        if (used > size) {
            size = (size == 0) ? 1 : size*2;
            temp = realloc(intervals, size);
            if (!temp) { perror("realloc"); exit(1); }
            intervals = temp;
        }

        line = arb_chop_by_delimiter((char **)&file_data, "\n");
        sscanf(line, "%u-%u,%u-%u", &(intervals[n].lo), &(intervals[n].hi), 
                                    &(intervals[n+1].lo), &(intervals[n+1].hi));
        n += 2;
    } while(*file_data != '\0');

    // resize storage to fit number of items
    temp = realloc(intervals, used);
    if (!temp) { perror("realloc"); exit(1); }
    intervals = temp;

    part_one(intervals, n);
    part_two(intervals, n);

    return 0;
}
