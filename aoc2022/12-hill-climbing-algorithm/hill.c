#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define CELL_WIDTH  16
#define CELL_HEIGHT 16
#define OFFY        180
#define SKIP_FRAMES 8

typedef struct {
   int x, y;
} Vec2i;

typedef struct {
    int height;
    U32 color;
} Cell;

typedef struct {
    int rows, cols;
    Vec2i start, end;
    int *data;
} Map;

typedef struct { 
    Vec2i key, value;
} Hashmap;

typedef struct {
    Vec2i pos;
    int priority;
} Priority_Position;


global U64 frames = 0;
global U32 colors[26] = {
    0x4d1b1b, 0x391b1f, 0x351b23, 0x311b27, 0x2d1b2b, 
    0x291b2f, 0x251b33, 0x211b37, 0x1d1b3b, 0x191b3f, 
    0x151b43, 0x111b47, 0x0d1b4b, 0x091b4f, 0x051b53, 
    0x011b57, 0x001b5b, 0x001b5f, 0x001b63, 0x001b67,
    0x001b6b, 0x001b6f, 0x001b73, 0x001b77, 0x001b7b, 
    0x001b7f
};

int vec2i_manhattan_dist(Vec2i a, Vec2i b) {
    return abs(b.y - a.y) + abs(b.x - a.x);
}

U32 saturate(U32 color) {
    float factor = 1.3;
    U32 r = factor * ((color >>  0) & 0xFF);
    U32 g = factor * ((color >>  8) & 0xFF);
    U32 b = factor * ((color >> 16) & 0xFF);
    r &= 0xFF; g &= 0xFF; b &= 0xFF;
    return r | (g << 8) | (b << 16);
}

void draw_map(Map map, Hashmap *came_from) {
    Vec2i pos;
    int i, j, x, y, height;
    U32 color;
    for (j=0; j<map.rows; ++j) {
        y = j * CELL_HEIGHT + OFFY;
        for (i=0; i<map.cols; ++i) {
            pos = (Vec2i){i, j};
            height = map.data[j*map.cols+i];
            x = i * CELL_WIDTH;
            color = colors[height];
            if (came_from && hmgeti(came_from, pos) != -1)
                color = saturate(color);
            drawbox(x, y, CELL_WIDTH, CELL_HEIGHT, color);
        }
    }
}

void draw_path(Vec2i start, Vec2i current, Hashmap *came_from) {
    int x, y;
    while (!(current.x == start.x && current.y == start.y)) {
        y = current.y * CELL_HEIGHT + OFFY;
        x = current.x * CELL_WIDTH;
        drawbox(x, y, CELL_WIDTH, CELL_HEIGHT, 0xDDBB00);
        current = hmget(came_from, current);
    }
    // draw start
    y = start.y * CELL_HEIGHT + OFFY;
    x = start.x * CELL_WIDTH;
    drawbox(x, y, CELL_WIDTH, CELL_HEIGHT, 0xDDBB00);
}

Priority_Position *priority_push(Priority_Position *queue, Vec2i item, int priority) {
    Priority_Position pp = { .pos = item, .priority = priority };

    int i;
    for (i = arrlen(queue) - 1; i >= 0; --i) {
        if (priority < queue[i].priority)
            break;
    }
    if (i < 0) i = 0;

    arrins(queue, i, pp);

    return queue;
}

int part_one(Map map, Vec2i start) {
    int steps = 0;
    bool reached_destination = false;

    // NOTE(shaw): using stb_ds dynamic array as a queue is dumb and slow
    // since it will shift the whole array every insert
    Priority_Position *frontier = NULL;
    Hashmap *came_from=NULL, *costs=NULL; 
    Vec2i current, neighbor, cost;
    int current_height, neighbor_height, priority;

    frontier = priority_push(frontier, start, 0);
    cost.x = 0;
    hmput(costs, start, cost);

    while (arrlen(frontier) > 0) {
        current = arrpop(frontier).pos;
        current_height = map.data[current.y * map.cols + current.x];
        
        if (current.x == map.end.x && current.y == map.end.y) {
            reached_destination = true;
            break;
        }

        Vec2i neighbors[4] = {
            (Vec2i){ current.x,   current.y+1 },
            (Vec2i){ current.x+1, current.y   },
            (Vec2i){ current.x,   current.y-1 },
            (Vec2i){ current.x-1, current.y   },
        };

        for (int i=0; i<4; ++i) {
            neighbor = neighbors[i];
            neighbor_height = map.data[neighbor.y * map.cols + neighbor.x];
            
            cost.x = hmget(costs, current).x + vec2i_manhattan_dist(current, neighbor);

            // if it can reach this neighbor and the cost to visit is less than
            // all previous paths to the neighbor
            if (neighbor.x >= 0 && neighbor.x < map.cols &&
                neighbor.y >= 0 && neighbor.y < map.rows &&
                neighbor_height <= current_height + 1 &&
                (hmgeti(costs, neighbor) == -1 || cost.x < hmget(costs, neighbor).x))
            {
                // visit it and add it to queue
                priority = cost.x + vec2i_manhattan_dist(neighbor, map.end);
                hmput(came_from, neighbor, current);
                frontier = priority_push(frontier, neighbor, priority);
                hmput(costs, neighbor, cost);

                draw_map(map, came_from);
                draw_path(start, current, came_from);
            }
        }
        if (frames % SKIP_FRAMES == 0)
            nextframe();
        ++frames;
    }

    if (reached_destination) {
        current = map.end;
        while (!(current.x == start.x && current.y == start.y)) {
            current = hmget(came_from, current);
            ++steps;
        }
    }

    draw_map(map, came_from);
    draw_path(start, map.end, came_from);
    nextframe();
    nextframe();

    arrfree(frontier);
    hmfree(came_from);
    hmfree(costs);

    return steps;
}

void part_two(Map map) {
    int min_steps = INT32_MAX;
    for (int j=0; j<map.rows; ++j)
    for (int i=0; i<map.cols; ++i) {
        if (map.data[j*map.cols+i] == 0) {
            int steps = part_one(map, (Vec2i){i,j});

            // zero means destination was not reached
            if (steps > 0)
                min_steps = Min(min_steps, steps);
        }
    }
    printf("part_two: %d\n", min_steps);
}

Map build_map(uint8_t *input) {
    Map map = {0};
    char *line;
    int row=0, col=0;
    size_t size = 0;
    int *new_data;

    do {
        col = 0;
        line = arb_chop_by_delimiter((char**)&input, "\n");
        map.cols = strlen(line);
        size += map.cols;

        new_data = realloc(map.data, size * sizeof(int));
        if (!new_data) { perror("realloc"); exit(1); }
        map.data = new_data;

        while (*line != '\0') {
            if (*line == 'S') {
                map.start = (Vec2i){ col, row };
                *line = 'a';
            }
            if (*line == 'E') {
                map.end = (Vec2i){ col, row };
                *line = 'z';
            }
            map.data[row*map.cols + col] = *line - 'a';
            ++line;
            ++col;
        }
        ++row;
    } while (*input != '\0');
    
    map.rows = row;

    return map;
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

    setupgif(0, 1, "hill.gif");

    Map map = build_map(file_data);

    draw_map(map, NULL);
    nextframe();

    int steps = part_one(map, map.start);
    printf("part_one: %d\n", steps);

    /*part_two(map);*/

    endgif();

    return 0;
}

