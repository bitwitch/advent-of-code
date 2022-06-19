#include <stdlib.h>
#include <iostream>
#include <vector>

#define STEPS 1000

using namespace std;

void print_grid(vector<vector<int>> &grid) {
    int height = grid.size();
    int width = grid[0].size();
    for (int j=0; j<height; ++j) {
        for (int i=0; i<width; ++i)
            cout << grid[j][i];
        cout << endl;
    }
}

int flash(vector<vector<int>> &grid, int row, int col, int width, int height) {
    int flashes = 1;
    for (int j = row-1; j <= row+1 && j < height; ++j) {
        if (j < 0) continue;
        for (int i=col-1; i<=col+1 && i<width; ++i) {
            if (i < 0) continue;
            if (++grid[j][i] == 10)
                flashes += flash(grid, j, i, width, height);
        }
    }
    return flashes;
}

int increment(vector<vector<int>> &grid, int width, int height) {
    int flashes = 0;
    for (int j=0; j<height; ++j) 
    for (int i=0; i<width; ++i) 
        if (++grid[j][i] == 10)
            flashes += flash(grid, j, i, width, height);
    return flashes;
}

void reset_flashed_octos(vector<vector<int>> &grid, int width, int height) {
    for (int j=0; j<height; ++j) 
    for (int i=0; i<width; ++i) 
        if (grid[j][i] > 9)
            grid[j][i] = 0;
}

int part_one(vector<vector<int>> &grid, int width, int height) {
    int flashes = 0;

    for (int step=0; step < STEPS; ++step) {
        flashes += increment(grid, width, height);
        reset_flashed_octos(grid, width, height);
    }

    return flashes;
}

int part_two(vector<vector<int>> &grid, int width, int height) {
    for (int step=0; step < STEPS; ++step) {
        int step_flashes = increment(grid, width, height);
        reset_flashed_octos(grid, width, height);
        cout << "After step " << step << ":" << endl; print_grid(grid); cout << endl;
        if (step_flashes == width * height)
            return step+1;
    }
    return -1;
}


int main(int argc, char **argv) {
    int width, height;
    cin >> width >> height;

    vector<vector<int>> grid;
    for (int j=0; j<height; ++j) {
        vector<int> row;
        for (int i=0; i<width; ++i) {
            int col;
            while ( (col = getchar()) == 10 || col == 13) {}
            col -= 48;
            row.push_back(col);
        }
        grid.push_back(row);
    }

    //int answer_one = part_one(grid, width, height);
    //cout << "Part One: " << answer_one << endl;

    int answer_two = part_two(grid, width, height);
    cout << "Part Two: " << answer_two << endl;

    return 0;
}
