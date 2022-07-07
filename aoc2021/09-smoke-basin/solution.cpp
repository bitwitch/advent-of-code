#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include "aoc2021.h"

struct Point {
    int x,y;
};

using namespace std;

void part_one(vector<vector<int>> &height_map) {
    int total_risk = 0;
    int rows = height_map.size(); 
    int cols = height_map[0].size();

    for (int j=0; j<rows; j++) 
    for (int i=0; i<cols; i++) {
        bool low = true;
        int current = height_map[j][i];

        // check top
        if (j > 0 && height_map[j-1][i] <= current)
            low = false;
        // check right
        if (i < cols-1 && height_map[j][i+1] <= current)
            low = false;
        // check bottom
        if (j < rows-1 && height_map[j+1][i] <= current)
            low = false;
        // check left
        if (i > 0 && height_map[j][i-1] <= current)
            low = false;

        if (low) 
            total_risk += current+1;
    }

    cout << "total risk: " << total_risk << endl;

}

void draw_basin(vector<vector<int>> &height_map) {
    int rows = height_map.size(); 
    int cols = height_map[0].size();

    int w = 1024 / cols;
    int h = 1024 / rows;

    clear();
    for (int j=0; j<rows; j++) 
    for (int i=0; i<cols; i++) {
        int color;
        switch (height_map[j][i]) {
            case  9: color = 0x0D2A40; break;
            case 10: color = 0xBDA773; break;
            default: color = 0x4F5C63; break;
        }
        int x0 = i * w;
        int x1 = x0 + w;
        int y0 = j * h;
        int y1 = y0 + h;
        drawrect(x0, y0, x1, y1, color); 
    }
    nextframe();
}

int fill_basin(vector<vector<int>> &height_map, int x, int y) {
    int count = 0;

    int rows = height_map.size(); 
    int cols = height_map[0].size();

    queue<Point> frontier;

    frontier.push({x,y});
    ++count;
    height_map[y][x] = 10;

    int frame = 0;
    while (!frontier.empty()) {
        Point p = frontier.front();
        frontier.pop();
        int i = p.x;
        int j = p.y;
        bool new_point = false;

        // check top
        if (j > 0 && height_map[j-1][i] < 9) {
            frontier.push({i,j-1});
            ++count;
            height_map[j-1][i] = 10;
            new_point = true;
        }

        // check right
        if (i < cols-1 && height_map[j][i+1] < 9) {
            frontier.push({i+1,j});
            ++count;
            height_map[j][i+1] = 10;
            new_point = true;
        }

        // check bottom
        if (j < rows-1 && height_map[j+1][i] < 9) {
            frontier.push({i,j+1});
            ++count;
            height_map[j+1][i] = 10;
            new_point = true;
        }

        // check left
        if (i > 0 && height_map[j][i-1] < 9) {
            frontier.push({i-1,j});
            ++count;
            height_map[j][i-1] = 10;
            new_point = true;
        }
            

        if (new_point && (frame % 4) == 0)
            draw_basin(height_map);
        ++frame;
    }

    return count;
}

void part_two(vector<vector<int>> &height_map) {
    setupgif(2, "fill_basins.gif");

    draw_basin(height_map);

    int rows = height_map.size(); 
    int cols = height_map[0].size();

    int max_1 = INT_MIN;
    int max_2 = INT_MIN;
    int max_3 = INT_MIN;

fill_basins:
    // loop through map to find an "unfilled" point
    for (int j=0; j<rows; j++) 
    for (int i=0; i<cols; i++) {
        if (height_map[j][i] < 9) {   
            int count = fill_basin(height_map, i, j);
            if (count > max_1) {
                max_3 = max_2;
                max_2 = max_1;
                max_1 = count;
            } else if (count > max_2) {
                max_3 = max_2;
                max_2 = count;
            } else if (count > max_3) {
                max_3 = count;
            } 
            goto fill_basins;
        }
    }
    endgif();
    unsigned long long result = max_1 * max_2 * max_3;
    printf("Largest three basins are %d %d %d\nTheir product is %lu\n", max_1, max_2, max_3, result);
}

int main(int argc, char **argv) {


    vector<vector<int>> height_map;

    char c;
    int i = 0;
    height_map.push_back(vector<int>());
    while(cin >> c) {
        height_map[i].push_back(c - '0');
        if (cin.peek() == '\n') {
            cin.ignore();
            if (cin.peek() == EOF) break;
            height_map.push_back(vector<int>());
            i++;
        }
    }

    if (argc == 2)
        part_two(height_map);
    else
        part_one(height_map);

    return 0;
}



