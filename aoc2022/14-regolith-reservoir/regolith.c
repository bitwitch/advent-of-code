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
    Vec2i *vertices; // dynamic array
} Rock;

typedef struct {
    Vec2i pos;
    bool settled;
} Particle;

Particle *particles = NULL; // dynamic array

Particle *spawn_particle(void) {
    Particle p = { .pos = {500, 0} };
    arrput(particles, p);
    return &particles[arrlen(particles) - 1];
}

bool rock_collide(Rock rock, Vec2i point) {
    for (int i=0; i < arrlen(rock.vertices) - 1; ++i) {
        Vec2i v1 = rock.vertices[i];
        Vec2i v2 = rock.vertices[i+1];
        bool horizontal = v1.y == v2.y;

        if (horizontal) {
            if (v1.x > v2.x) {
                Vec2i tmp = v1;
                v1 = v2;
                v2 = tmp;
            }

            if (point.y == v1.y && point.x >= v1.x && point.x <= v2.x)
                return true;

        } else { // vertical
            if (v1.y > v2.y) {
                Vec2i tmp = v1;
                v1 = v2;
                v2 = tmp;
            }

            if (point.x == v1.x && point.y >= v1.y && point.y <= v2.y)
                return true;
        }
    }

    return false;
}

bool collide(Rock *rocks, Vec2i point) {
    for (int i=0; i<arrlen(rocks); i++)
        if (rock_collide(rocks[i], point))
            return true;
    for (int i=0; i<arrlen(particles); i++)
        if (point.x == particles[i].pos.x && point.y == particles[i].pos.y)
            return true;
    return false;
}

void move_particle(Particle *p, Rock *rocks) {
    Vec2i bottom       = { p->pos.x,   p->pos.y+1 };
    Vec2i bottom_left  = { p->pos.x-1, p->pos.y+1 };
    Vec2i bottom_right = { p->pos.x+1, p->pos.y+1 };

    if (!collide(rocks, bottom))
        p->pos = bottom;
    else if (!collide(rocks, bottom_left))
        p->pos = bottom_left;
    else if (!collide(rocks, bottom_right))
        p->pos = bottom_right;
    else
        p->settled = true;
}

bool in_the_void(Particle *p) {
    if (p->pos.y > 500) {
        printf("into the void\n");
        return true;
    }
    return false;
}

bool simulate(Rock *rocks, int *count) {
    static Particle *p = NULL;

    if (p == NULL || p->settled) {
        p = spawn_particle();
        *count += 1;
    } else {
        move_particle(p, rocks);
        if (in_the_void(p))
            return false;
    }
    return true;
}

void part_one(Rock *rocks) {
    int count = 0;
    for (bool running = true; running; running = simulate(rocks, &count));
    printf("part_one: %d\n", count-1);
}

void part_two(Rock *rocks) {
    (void)rocks;
    printf("part_two: %d\n", 696969);
}


Rock *parse_rocks(U8 *input) {
    Rock *rocks = NULL;
    do {
        char *line = arb_chop_by_delimiter((char**)&input, "\n");
        Rock rock = {0};

        do {
            char *pair = arb_chop_by_delimiter((char**)&line, " -> ");
            int x, y;
            if (sscanf(pair, "%d,%d", &x, &y) == 2) {
                arrput(rock.vertices, ((Vec2i){x,y}));
            } 
        } while (*line != '\0');

        arrput(rocks, rock);
    } while (*input != '\0');
    return rocks;
}

void print_rocks(Rock *rocks) {
    printf("%ld rocks\n", arrlen(rocks));
    for (int i=0; i<arrlen(rocks); ++i) {
        printf("rock %d\n", i);
        for (int j=0; j<arrlen(rocks[i].vertices); ++j) {
            printf("\t(%d,%d)\n", rocks[i].vertices[j].x, rocks[i].vertices[j].y);
        }
    }
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

    Rock *rocks = parse_rocks(file_data);
    /*print_rocks(rocks);*/

    part_one(rocks);
    part_two(rocks);

    return 0;
}
