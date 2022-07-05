#include <stdio.h>

int die = 1;
int rolls = 0;

int main(void) {
    int players[2] = {4, 1};
    int scores[2] = {0, 0};

    int player = 0;
    while (scores[0] < 1000 && scores[1] < 1000) {
        int move = die + die+1 + die+2;
        die += 3;
        rolls += 3;
        players[player] = ((players[player] + move - 1) % 10) + 1;
        scores[player] += players[player];
        printf("player %d rolls %d and moves to space %d for total score of %d\n",
            player+1, move, players[player], scores[player]);
        player = !player;
    }

    printf("Part one result: %d\n", scores[player] * rolls);

    return 0;
}
