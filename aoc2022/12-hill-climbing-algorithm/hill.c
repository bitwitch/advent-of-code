#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

typedef struct {
   int x, y; 
} Vec2i;

typedef struct {
    int rows, cols;
    Vec2i start, end;
    int *data;
} Map;

typedef struct {
    Vec2i pos;
    int data;
} Node;

int part_one(Map map, Vec2i start) {
    /*
    for (int j=0; j<map.rows; ++j) {
        for (int i=0; i<map.cols; ++i) {
            printf("%c ", map.data[j*map.cols+i] + 'a');
        }
        printf("\n");
    }
    */

    int steps = 0;

    // NOTE(shaw): using stb_ds dynamic array as a queue is dumb and slow
    Vec2i *frontier = NULL;
    struct { Vec2i key, value; } *came_from = NULL; // hashmap
    Vec2i current, neighbor;
    int current_height, neighbor_height;

    arrput(frontier, start);

    while (arrlen(frontier) > 0) {
        current = arrpop(frontier);
        current_height = map.data[current.y * map.cols + current.x];
        
        if (current.x == map.end.x && current.y == map.end.y) {
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

            // if it can reach this neighbor and the neighbor hasn't already been visited
            if (neighbor.x >= 0 && neighbor.x < map.cols &&
                neighbor.y >= 0 && neighbor.y < map.rows &&
                neighbor_height <= current_height + 1 &&
                hmgeti(came_from, neighbor) == -1)
            {
                // visit it and add it to queue
                hmput(came_from, neighbor, current);
                arrins(frontier, 0, neighbor);
            }
        }
    }

    current = map.end;
    while (!(current.x == start.x && current.y == start.y)) {
        current = hmget(came_from, current);
        ++steps;
    }

    return steps;
}

void part_two(Map map) {
    int min_steps = INT32_MAX;
    for (int j=0; j<map.rows; ++j)
    for (int i=0; i<map.cols; ++i) {
        if (map.data[j*map.cols+i] == 0) {
            int steps = part_one(map, (Vec2i){i,j});
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

    // TODO: build a graph representing the height map
    //
    Map map = build_map(file_data);

    int steps = part_one(map, map.start);
    printf("part_one: %d\n", steps);

    part_two(map);

    return 0;
}
