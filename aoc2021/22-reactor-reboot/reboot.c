#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CUBOIDS 65536
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

typedef struct  {
    int min, max;
} Range;

typedef struct  {
    bool on;
    Range x, y, z;
} Cuboid;

void print_cuboid(Cuboid *c) {
    printf("x=%d..%d, y=%d..%d, z=%d..%d\n",
        c->x.min, c->x.max,
        c->y.min, c->y.max,
        c->z.min, c->z.max);
}

void part_one_naive(Cuboid steps[], int step_count) {
    static int grid[101][101][101] = {0};

    for (int i=0; i<step_count; ++i) {
        Cuboid step = steps[i];
        for (int x=MAX(step.x.min, -50); x<=MIN(step.x.max, 50); ++x)
        for (int y=MAX(step.y.min, -50); y<=MIN(step.y.max, 50); ++y)
        for (int z=MAX(step.z.min, -50); z<=MIN(step.z.max, 50); ++z)
            grid[x+50][y+50][z+50] = step.on;


        int count = 0;
        for (int x=0; x<101; ++x)
        for (int y=0; y<101; ++y)
        for (int z=0; z<101; ++z)
            if (grid[x][y][z]) ++count;

        printf("After step %d: %d cubes are turned on\n", i, count);
    }

    /*int count = 0;*/
    /*for (int x=0; x<101; ++x)*/
    /*for (int y=0; y<101; ++y)*/
    /*for (int z=0; z<101; ++z)*/
        /*if (grid[x][y][z]) ++count;*/

    /*printf("After initialization procedure %d cubes are turned on\n", count);*/
}


bool cuboids_intersect(Cuboid a, Cuboid b, Cuboid *intersection) {
    intersection->x.min = MAX(a.x.min, b.x.min);
    intersection->x.max = MIN(a.x.max, b.x.max);
    intersection->y.min = MAX(a.y.min, b.y.min);
    intersection->y.max = MIN(a.y.max, b.y.max);
    intersection->z.min = MAX(a.z.min, b.z.min);
    intersection->z.max = MIN(a.z.max, b.z.max);

    return (intersection->x.min <= intersection->x.max &&
            intersection->y.min <= intersection->y.max &&
            intersection->z.min <= intersection->z.max);
}

void split_cuboid_on_intersection(Cuboid cuboid, Cuboid intersection, 
                                  Cuboid split[6], int *split_count) 
{
    /* left and right are over full y and z range of cuboid */
    Cuboid left = {
        .on = true,
        .x = {.min = cuboid.x.min, .max = intersection.x.min-1},
        .y = {.min = cuboid.y.min, .max = cuboid.y.max},
        .z = {.min = cuboid.z.min, .max = cuboid.z.max}
    };
    Cuboid right = {
        .on = true,
        .x = {.min = intersection.x.max+1, .max = cuboid.x.max},
        .y = {.min = cuboid.y.min, .max = cuboid.y.max},
        .z = {.min = cuboid.z.min, .max = cuboid.z.max}
    };

    /* top and bottom are over full z range of cuboid, but only x over range of intersection */
    Cuboid top = {
        .on = true,
        .x = {.min = intersection.x.min, .max = intersection.x.max},
        .y = {.min = intersection.y.max+1, .max = cuboid.y.max},
        .z = {.min = cuboid.z.min, .max = cuboid.z.max}
    };
    Cuboid bottom = {
        .on = true,
        .x = {.min = intersection.x.min, .max = intersection.x.max},
        .y = {.min = cuboid.y.min, .max = intersection.y.min-1},
        .z = {.min = cuboid.z.min, .max = cuboid.z.max}
    };

    /* front and back are only over x and y range of intersection */
    Cuboid front = {
        .on = true,
        .x = {.min = intersection.x.min, .max = intersection.x.max},
        .y = {.min = intersection.y.min, .max = intersection.y.max},
        .z = {.min = intersection.z.max+1, .max = cuboid.z.max}
    };
    Cuboid back = {
        .on = true,
        .x = {.min = intersection.x.min, .max = intersection.x.max},
        .y = {.min = intersection.y.min, .max = intersection.y.max},
        .z = {.min = cuboid.z.min, .max = intersection.z.min-1}
    };

    int split_index = 0;

    if (left.x.min <= left.x.max)
        split[split_index++] = left;
    if (right.x.min <= right.x.max)
        split[split_index++] = right;
    if (top.y.min <= top.y.max)
        split[split_index++] = top;
    if (bottom.y.min <= bottom.y.max)
        split[split_index++] = bottom;
    if (front.z.min <= front.z.max)
        split[split_index++] = front;
    if (back.z.min <= back.z.max)
        split[split_index++] = back;

    *split_count = split_index;
}


void part_two(Cuboid steps[], int step_count) {
    Cuboid *on_cuboids = calloc(MAX_CUBOIDS, sizeof(Cuboid));

    memset(on_cuboids, 0, step_count * sizeof(Cuboid));
    uint64_t on_cuboids_count = 0;

    Cuboid *cuboids_to_add = malloc(MAX_CUBOIDS * sizeof(Cuboid));
    uint64_t cuboids_to_add_count;

    uint64_t cuboids_to_remove_count;

    for (int step_i=0; step_i<step_count; ++step_i) {
    /*for (int i=0; i<2; ++i) {*/
        Cuboid step = steps[step_i];
        bool found_intersection = false;
        cuboids_to_add_count = 0;
        cuboids_to_remove_count = 0;

        /* check on_cuboids for overlapping regions */
        for (int j=0; j<on_cuboids_count; ++j) {
            Cuboid other = on_cuboids[j];
            Cuboid intersection;
            if (cuboids_intersect(step, other, &intersection)) {
                found_intersection = true;

                /* split other into new parts */    
                Cuboid other_split[6];
                int split_count;
                split_cuboid_on_intersection(other, intersection, other_split, &split_count);

                /* add the new non-overlapping cuboids from other to on_cuboids */
                for (int k=0; k<split_count; ++k) {
                    assert(cuboids_to_add_count < MAX_CUBOIDS);
                    cuboids_to_add[cuboids_to_add_count++] = other_split[k];
                }

                /* remove other from on_cuboids */
                on_cuboids[j].on = false;
                ++cuboids_to_remove_count;
            }
        }

        /*printf("to_remove:\n");*/
        if (found_intersection) {
            /* remove cuboids */
            int left = 0;
            int right = on_cuboids_count-1;
            while (left <= right) {
                if (on_cuboids[left].on) {
                    ++left;
                    continue;
                }

                if (!on_cuboids[right].on) {
                    --right;
                    continue;
                }

                /*print_cuboid(&on_cuboids[left]);*/

                on_cuboids[left++] = on_cuboids[right--];
            }
            on_cuboids_count -= cuboids_to_remove_count;

            /* add new cuboids */
            /*printf("to_add:\n");*/
            for (int i=0; i<cuboids_to_add_count; ++i) {
                assert(on_cuboids_count < MAX_CUBOIDS);
                /*print_cuboid(&cuboids_to_add[i]);*/
                on_cuboids[on_cuboids_count++] = cuboids_to_add[i];
            }

        }

        /* add new step */
        if (step.on) {
            /*printf("to_add:\n");*/
            /*print_cuboid(&step);*/
            assert(cuboids_to_add_count < MAX_CUBOIDS);
            on_cuboids[on_cuboids_count++] = step;
        } 

        uint64_t count = 0;
        for (int i=0; i<on_cuboids_count; ++i) {
            Cuboid c = on_cuboids[i];
            /*print_cuboid(&c);*/
            count += (c.x.max+1 - c.x.min) * (c.y.max+1 - c.y.min) * (c.z.max+1 - c.z.min);
        }

        printf("After step %d: %llu cubes are turned on\n", step_i, count);
    }

    printf("Found %llu cuboids\n", on_cuboids_count);
    uint64_t count = 0;
    for (int i=0; i<on_cuboids_count; ++i) {
        Cuboid c = on_cuboids[i];
        count += (c.x.max+1 - c.x.min) * (c.y.max+1 - c.y.min) * (c.z.max+1 - c.z.min);
    }

    printf("After reboot sequence %llu cubes are turned on\n", count);

    free(cuboids_to_add);

}

int main(void) {
    int step_count = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    FILE *fp = fopen("small_input.txt", "r");

    /* pass one, get step count */
    while ((nread = getline(&line, &len, fp)) != -1)
        ++step_count;        

    printf("found %d steps\n", step_count);

    /* pass two, get cuboids */
    Cuboid steps[step_count];
    memset(steps, 0, step_count * sizeof(Cuboid));

    rewind(fp);

    int i = 0;
    while ((nread = getline(&line, &len, fp)) != -1) {
        steps[i].on = strstr(line, "on") ? 1 : 0;

        char *cursor = index(line, 'x');
        sscanf(cursor, "x=%d..%d,y=%d..%d,z=%d..%d", 
            &steps[i].x.min, &steps[i].x.max,
            &steps[i].y.min, &steps[i].y.max,
            &steps[i].z.min, &steps[i].z.max);

        ++i;
    }

    fclose(fp);

    part_one_naive(steps, step_count);
    printf("\n\n");
    part_two(steps, step_count);

    return 0;
}

