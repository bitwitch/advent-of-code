#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define KNOT_COUNT 10

typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_COUNT
} Direction;

typedef struct {
    Direction direction;
    int distance;
} Movement;

typedef struct {
    int x, y;
} Vec2i;

void tail_follow(Vec2i *tail, Vec2i head) {
    // if two away directly up, down, left, or right step one in that direction
    if (tail->x == head.x) { 
        if (head.y - tail->y >  1) ++tail->y;
        if (head.y - tail->y < -1) --tail->y;
    } else if (tail->y == head.y) {
        if (head.x - tail->x >  1) ++tail->x;
        if (head.x - tail->x < -1) --tail->x;

    // else if head and tail arent touching and arent in same row or column, 
    // move one step diagonally to keep up
    } else if (abs(head.x - tail->x) > 1 || abs(head.y - tail->y) > 1 ) {
        tail->x += head.x > tail->x ? 1 : -1;
        tail->y += head.y > tail->y ? 1 : -1;
    }
}

void step(Vec2i *pos, Direction direction) {
    switch (direction) {
        case DIRECTION_UP:    ++pos->y; break;
        case DIRECTION_DOWN:  --pos->y; break;
        case DIRECTION_LEFT:  --pos->x; break;
        case DIRECTION_RIGHT: ++pos->x; break;
        default: 
            fprintf(stderr, "move: Unknown direction %d\n", direction);
            break;
    }
}

void part_one(Movement *movements) {
    int i, j;
    Movement m;
    Vec2i head = {0, 0};
    Vec2i tail = {0, 0};
    struct { Vec2i key; bool value; } *visited = NULL;

    for (i=0; i<arrlen(movements); ++i) {
        m = movements[i];
        for (j=0; j<m.distance; ++j) {
            step(&head, m.direction);
            tail_follow(&tail, head);
            hmput(visited, tail, true);
        }
    }
    printf("part_one: %ld\n", hmlen(visited));
}

void part_two(Movement *movements) {
    int i, j, k;
    Movement m;
    Vec2i knots[KNOT_COUNT] = {0};
    Vec2i *head = &knots[0];
    struct { Vec2i key; bool value; } *visited = NULL;

    for (i=0; i<arrlen(movements); ++i) {
        m = movements[i];
        for (j=0; j<m.distance; ++j) {
            step(head, m.direction);
            for (k=1; k<KNOT_COUNT; ++k) {
                tail_follow(&knots[k], knots[k-1]);
            }
            hmput(visited, knots[KNOT_COUNT-1], true);
        }
    }
    printf("part_two: %ld\n", hmlen(visited));
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
    Movement *movements = NULL; // dynamic array
    Movement m; 
    char dir = '\0';

    do {
        line = arb_chop_by_delimiter((char **)&file_data, "\n");
        sscanf(line, "%c %d", &dir, &m.distance);
        if (dir == 'U')      m.direction = DIRECTION_UP;
        else if (dir == 'D') m.direction = DIRECTION_DOWN;
        else if (dir == 'L') m.direction = DIRECTION_LEFT;
        else if (dir == 'R') m.direction = DIRECTION_RIGHT;
        else { fprintf(stderr, "Unknown direction %c\n", dir); exit(1); }
        arrput(movements, m);
    } while (*file_data != '\0');

    part_one(movements);
    part_two(movements);

    return 0;
}
