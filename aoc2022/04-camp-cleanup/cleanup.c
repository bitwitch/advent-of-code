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

void visualize_intervals_isolated(Interval a, Interval b) {
    int half_dim = 512;
    int grow_iterations = 10, emphasize_iterations = 3;
    int margin_left = 1;
    float grow_rate = 5.0;
    float unit = 20.0;
    float max_unit = unit + (grow_rate * grow_iterations);

    int min_x = Min(a.lo, b.lo);
    int max_x = Max(a.hi, b.hi);
    int max_dist = (max_x - min_x) * max_unit;

    float scale = (max_dist > 2*half_dim) 
        ? (2*half_dim) / (float)max_dist 
        : 1.0;

    unit *= scale;
    grow_rate *= scale;

    struct {
        int x, y, width, color;
    } va, vb;

    int i, lo, hi;
    Interval overlap = {0};
    int overlap_color = 0x00FFFF;
    local_persist int count = 0;

    clear();

    // draw some frames of the two intervals growing on screen
    for (i=0; i<grow_iterations; ++i) {
        va.y = half_dim - 2*unit;
        va.width = (a.hi - a.lo) * unit;
        va.x = (margin_left + a.lo - min_x) * unit;
        va.color = 0xDDDDFF;

        vb.y = half_dim + unit;
        vb.width = (b.hi - b.lo) * unit;
        vb.x = (margin_left + b.lo - min_x) * unit;
        vb.color = 0xFFDDDD;
        
        /*drawrect(va.x, va.y, va.x+va.width, va.y+unit, va.color);*/
        /*drawline(ax, ay, va.x, va.y+unit, 2, 0x000000);*/
        strokerect(va.x, va.y, va.x+va.width, va.y+unit, 2, va.color);

        /*drawrect(vb.x, vb.y, vb.x+vb.width, vb.y+unit, vb.color);*/
        /*drawline(bx, by, vb.x, vb.y, 2, 0x000000);*/
        strokerect(vb.x, vb.y, vb.x+vb.width, vb.y+unit, 2, vb.color);

        nextframe(0);
        clear();

        unit += grow_rate;
    }

    unit -= grow_rate; // undo last grow

    //
    // emphasize the overlapping sections
    //
    if (a.lo >= b.lo && a.lo <  b.hi) overlap.lo = a.lo;
    if (a.hi >  b.lo && a.hi <= b.hi) overlap.hi = a.hi;
    if (b.lo >= a.lo && b.lo <  a.hi) overlap.lo = b.lo;
    if (b.hi >  a.lo && b.hi <= a.hi) overlap.hi = b.hi;

    if (overlap.lo < overlap.hi) ++count;

    for (i=0; i<emphasize_iterations; ++i) {
        strokerect(va.x, va.y, va.x+va.width, va.y+unit, 2, va.color);
        strokerect(vb.x, vb.y, vb.x+vb.width, vb.y+unit, 2, vb.color);

        if (overlap.lo < overlap.hi) {
            lo = (margin_left + overlap.lo - min_x) * unit;
            hi = (margin_left + overlap.hi - min_x) * unit;
            drawrect(lo, vb.y, hi, vb.y+unit, overlap_color);
            drawrect(lo, va.y, hi, va.y+unit, overlap_color);
            drawstringf(512-30, 200, 4, 0xFFFFFF, "%d", count);
        }

        nextframe(0);
    }
}

int cum_x, cum_y;
void visualize_intervals_cummulative(Interval a, Interval b) {
    local_persist int frames = 0;
    int gif_width = 1024;
    int gif_height = 1024;
    int unit = 5;
    int y_margin = 1;
    int height = 1;
    int overlap_color = 0x00FFFF;
    Interval overlap;

    // clear the two rows, there might be previous rows drawn underneath
    drawrect(cum_x*unit, cum_y*unit, cum_x*unit + gif_width/2, (cum_y + 2*height)*unit, 0x000000);

    // draw the new rows
    drawrect((cum_x + a.lo)*unit, cum_y*unit, (cum_x+ a.hi)*unit, (cum_y+height)*unit, 0x00FF00);
    drawrect((cum_x + b.lo)*unit, (cum_y+y_margin)*unit, (cum_x + b.hi)*unit, (cum_y+y_margin+height)*unit, 0x0000FF);

    //
    // emphasize the overlapping sections
    //
    if (a.lo >= b.lo && a.lo <  b.hi) overlap.lo = a.lo;
    if (a.hi >  b.lo && a.hi <= b.hi) overlap.hi = a.hi;
    if (b.lo >= a.lo && b.lo <  a.hi) overlap.lo = b.lo;
    if (b.hi >  a.lo && b.hi <= a.hi) overlap.hi = b.hi;

    if (overlap.lo < overlap.hi) {
        drawrect((cum_x + overlap.lo)*unit, cum_y*unit, 
                (cum_x + overlap.hi)*unit, (cum_y+height)*unit, 
                overlap_color);
        drawrect((cum_x + overlap.lo)*unit, (cum_y+y_margin)*unit, 
                (cum_x + overlap.hi)*unit, (cum_y+y_margin+height)*unit, 
                overlap_color);
    }


    cum_y += 2 * y_margin;
    if (cum_y * unit > gif_height) {
        cum_y = 0;
        cum_x = (cum_x == 0) ? 100 : 0;
    }

    if ((frames++ % 5) == 0)
        nextframe(0);
}

void draw_first_n_pairs(Interval *intervals, int n) {
    cum_x = 0;
    cum_y = 0;

    Interval a, b;

    clear();
    for (int i=0; i<n; ++i) {
        a = intervals[i*2];
        b = intervals[i*2+1];
        visualize_intervals_cummulative(a, b);
    }
}


void part_two(Interval *intervals, int intervals_count) {
    int count = 0;
    int i;
    Interval a, b;
    int final_count_iterations = 3;
    int pairs_show_isolated = 5;

    for (i=0; i<intervals_count-1; i+=2) {
        a = intervals[i];
        b = intervals[i+1];

        if (i < pairs_show_isolated*2) {
            /*visualize_intervals_isolated(a, b);*/
        } else {
            if (i == pairs_show_isolated*2)
                draw_first_n_pairs(intervals, pairs_show_isolated);
            visualize_intervals_cummulative(a, b);
        }

        if ((a.lo >= b.lo && a.lo <  b.hi) ||
            (a.hi >  b.lo && a.hi <= b.hi) ||
            (b.lo >= a.lo && b.lo <  a.hi) ||
            (b.hi >  a.lo && b.hi <= a.hi))
        {
            ++count;

            // display current overlap count
            if (i >= pairs_show_isolated*2) {
                drawrect(512-35, 200, 512+65, 250, 0x000000);
                drawstringf(512-30, 200, 4, 0xFFFFFF, "%d", count);
                nextframe(0);
            }
        }
    }

    clear();

    // display final overlap count
    for (i=0; i<final_count_iterations; ++i) {
        drawstringf(512-30, 200, 4, 0xFFFFFF, "%d", count);
        nextframe(0);
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

        // increment hi since we want to think of a single "section" as having
        // area rather than a point on x axis
        ++intervals[n].hi;
        ++intervals[n+1].hi;

        n += 2;
    } while(*file_data != '\0');

    // resize storage to fit number of items
    temp = realloc(intervals, used);
    if (!temp) { perror("realloc"); exit(1); }
    intervals = temp;

    setupgif(0, 2, "cleanup.gif");

    part_one(intervals, n);
    part_two(intervals, n);

    endgif();

    return 0;
}
