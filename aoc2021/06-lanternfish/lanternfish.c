#include <stdio.h>
#include <stdint.h>

#define STEPS 256

int main(void) {
    uint64_t fish[9] = {0};

    char c;
    FILE *fp = fopen("input", "r");
    while ((c = getc(fp)) != EOF) {
        if (c == ',') continue;
        fish[c-48] += 1;    
    }
    fclose(fp);

    for (int i=0; i<STEPS; ++i) {
        uint64_t zeros = fish[0];
        for (int f=0; f<8; ++f)
            fish[f] = fish[f+1];
        fish[6] += zeros;
        fish[8] = zeros;
    }

    uint64_t count = 0;
    for (int f=0; f<9; ++f)
        count += fish[f];

    printf("There are %llu fish after %d days\n", count, STEPS);

    return 0;

}
