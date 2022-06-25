#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int x, y;
} Vec2;

typedef struct {
    int x, y, w, h;
} Rect;

#define MAX_STEPS 2000
#define MAX_START_MAGNITUDE 1000

bool point_in_rect(Vec2 position, Rect target) {
    return ((position.x >= target.x) &&
            (position.x < target.x+target.w) &&
            (position.y <= target.y) &&
            (position.y > target.y-target.h));
}

int main(void) {
    Rect target = {288, -50, 43, 47};
    /*Rect target = {20, -5, 11, 6};*/

    Vec2 position = {.x=0, .y=0};
    Vec2 start_velocity = {.x=0, .y=0};
    bool solution = false;
    int max_height = 0;
    int count = 0;

    int dir_x = target.x > 0 ? 1 : -1;

    for (int y=MAX_START_MAGNITUDE; y>-MAX_START_MAGNITUDE; --y)
    for (int x=MAX_START_MAGNITUDE; x>0; --x)
    {
        position = (Vec2){0,0};
        max_height = 0;
        start_velocity = (Vec2){x * dir_x, y};
        Vec2 velocity = start_velocity;

        for (int step=0; step<MAX_STEPS; ++step) {
            position.x += velocity.x;
            position.y += velocity.y;

            if (position.y > max_height)
                max_height = position.y;

            if (point_in_rect(position, target)) {
                solution = max_height;
                ++count;
                break;
            }

            if (velocity.x != 0) {
                int drag = velocity.x > 0 ? -1 : 1;
                velocity.x += drag;
            }
            --velocity.y;
        }
    }
 
    printf("%d solutions found\n", count);

    return 0;
}
