#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

typedef struct  {
    int min, max;
} Range;

typedef struct  {
    bool on;
    Range x, y, z;
} Cuboid;

void part_one_naive(Cuboid *steps, int step_count) {
    static int grid[101][101][101] = {0};

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

int main(void) {
    int step_count = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    FILE *fp = fopen("input.txt", "r");

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

    return 0;
}
