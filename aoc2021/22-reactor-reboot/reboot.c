#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CUBOIDS 2048
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

typedef struct  {
    int min, max;
} Range;

typedef struct  {
    bool on;
    Range x, y, z;
} Cuboid;

void part_one_naive(Cuboid steps[], int step_count) {
    static int grid[101][101][101] = {0};

    /*for (int i=0; i<2; ++i) {*/
    for (int i=0; i<step_count; ++i) {
        Cuboid step = steps[i];
        for (int x=MAX(step.x.min, -50); x<=MIN(step.x.max, 50); ++x)
        for (int y=MAX(step.y.min, -50); y<=MIN(step.y.max, 50); ++y)
        for (int z=MAX(step.z.min, -50); z<=MIN(step.z.max, 50); ++z)
            grid[x+50][y+50][z+50] = step.on;
    }

    int count = 0;
    for (int x=0; x<101; ++x)
    for (int y=0; y<101; ++y)
    for (int z=0; z<101; ++z)
        if (grid[x][y][z]) ++count;

    printf("After initialization procedure %d cubes are turned on\n", count);
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



/* NOTE(shaw): for a first attempt, just doing a very simple split on all axes,
 * so potentially 27 new cuboids whenever a cuboid is split 
 * 
 * If this ends up not being sufficient, could try doing something smarter like
 * handling each specific case of where the intersection is to split the cuboid
 * into as few parts as necessary */
void split_cuboid_on_intersection(Cuboid cuboid, Cuboid intersection, Cuboid split[27]) {
    Range x_split[3] = {
        { .min = cuboid.x.min,       .max = intersection.x.min-1 },
        { .min = intersection.x.min, .max = intersection.x.max },
        { .min = intersection.x.max+1, .max = cuboid.x.max }
    };
    Range y_split[3] = {
        { .min = cuboid.y.min,       .max = intersection.y.min-1 },
        { .min = intersection.y.min, .max = intersection.y.max },
        { .min = intersection.y.max+1, .max = cuboid.y.max }
    };
    Range z_split[3] = {
        { .min = cuboid.z.min,       .max = intersection.z.min-1 },
        { .min = intersection.z.min, .max = intersection.z.max },
        { .min = intersection.z.max+1, .max = cuboid.z.max }
    };

    int split_index = 0;

    for (int i=0; i<3; ++i)
    for (int j=0; j<3; ++j)
    for (int k=0; k<3; ++k)
        split[split_index++] = (Cuboid){
            .on = cuboid.on,
            .x  = x_split[i],
            .y  = y_split[j],
            .z  = z_split[k]
        };
}


void part_two(Cuboid steps[], int step_count) {
    /*Cuboid on_cuboids[step_count];*/
    Cuboid *on_cuboids = calloc(MAX_CUBOIDS, sizeof(Cuboid));

    memset(on_cuboids, 0, step_count * sizeof(Cuboid));
    int on_cuboids_count = 0;

    /*Cuboid cuboids_to_add[MAX_CUBOIDS];*/
    Cuboid *cuboids_to_add = malloc(MAX_CUBOIDS * sizeof(Cuboid));
    int cuboids_to_add_count;

    /*int cuboids_to_remove[MAX_CUBOIDS];*/
    int *cuboids_to_remove = malloc(MAX_CUBOIDS * sizeof(int));
    int cuboids_to_remove_count;

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
                Cuboid other_split[27];
                split_cuboid_on_intersection(other, intersection, other_split);

                /* add the new non-overlapping cuboids from other to on_cuboids */
                /*APPEND_NON_INTERSECTING_TO_ADD(other_split, intersection);*/
                for (int k=0; k<27; ++k) {
                    Cuboid c = other_split[k];
                    if (c.x.min <= c.x.max && c.y.min <= c.y.max && c.z.min <= c.z.max &&
                        !(c.x.min == intersection.x.min && c.x.max == intersection.x.max && 
                          c.y.min == intersection.y.min && c.y.max == intersection.y.max && 
                          c.z.min == intersection.z.min && c.z.max == intersection.z.max))
                    {
                        assert(cuboids_to_add_count < MAX_CUBOIDS);
                        cuboids_to_add[cuboids_to_add_count++] = c;
                    }
                }

                /* remove other from on_cuboids */
                assert(cuboids_to_remove_count < MAX_CUBOIDS);
                cuboids_to_remove[cuboids_to_remove_count++] = j;

                if (step.on) {
                    /* split step into new parts */
                    Cuboid step_split[27];
                    split_cuboid_on_intersection(step, intersection, step_split);
                    /* add the new non-overlapping cuboids from step to on_cuboids */
                    for (int k=0; k<27; ++k) {
                        Cuboid c = step_split[k];
                        if (c.x.min <= c.x.max && c.y.min <= c.y.max && c.z.min <= c.z.max &&
                                !(c.x.min == intersection.x.min && c.x.max == intersection.x.max && 
                                    c.y.min == intersection.y.min && c.y.max == intersection.y.max && 
                                    c.z.min == intersection.z.min && c.z.max == intersection.z.max))
                        {
                            assert(cuboids_to_add_count < MAX_CUBOIDS);
                            cuboids_to_add[cuboids_to_add_count++] = c;
                        }
                    }

                    /* add the 1 overlapping cuboid to on_cuboids */
                    assert(cuboids_to_add_count < MAX_CUBOIDS);
                    cuboids_to_add[cuboids_to_add_count++] = intersection;
                } 
            }
        }

        if (found_intersection) {
            /* remove cuboids */
            for (int i=0; i<cuboids_to_remove_count; ++i) {
                int index_to_remove = cuboids_to_remove[i];
                on_cuboids[index_to_remove] = on_cuboids[--on_cuboids_count];
            }

            /* add new cuboids */
            for (int i=0; i<cuboids_to_add_count; ++i) {
                assert(on_cuboids_count < MAX_CUBOIDS);
                on_cuboids[on_cuboids_count++] = cuboids_to_add[i];
            }

        } else {
            /* no overlapping regions, just add step to on_cuboids */
            assert(on_cuboids_count < MAX_CUBOIDS);
            on_cuboids[on_cuboids_count++] = step;
        }
    }

    printf("Found %d cuboids\n", on_cuboids_count);
    uint64_t count = 0;
    for (int i=0; i<on_cuboids_count; ++i) {
        Cuboid c = on_cuboids[i];
        /*printf("x=%d..%d, y=%d..%d, z=%d..%d\n",*/
            /*c.x.min, c.x.max,*/
            /*c.y.min, c.y.max,*/
            /*c.z.min, c.z.max);*/
        count += (c.x.max+1 - c.x.min) * (c.y.max+1 - c.y.min) * (c.z.max+1 - c.z.min);
    }

    printf("After reboot sequence %lu cubes are turned on\n", count);


     free(cuboids_to_add);
     free(cuboids_to_remove);

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
       if (strstr(line, "on"))
           steps[i].on = true;

        char *cursor = index(line, 'x');
        sscanf(cursor, "x=%d..%d,y=%d..%d,z=%d..%d", 
            &steps[i].x.min, &steps[i].x.max,
            &steps[i].y.min, &steps[i].y.max,
            &steps[i].z.min, &steps[i].z.max);

        ++i;
    }

    fclose(fp);

    part_one_naive(steps, step_count);
    part_two(steps, step_count);

    return 0;
}
