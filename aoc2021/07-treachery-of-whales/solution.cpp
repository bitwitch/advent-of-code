#include <cstdint>
#include <iostream>
#include <limits.h>
#include <string>
#include <vector>

using namespace std;

uint64_t summation(int n) {
    return ((uint64_t)n * ((uint64_t)n+1)) / 2;
}

void part_one(vector<int> &positions) {
    int mid = positions[positions.size() / 2];
    int total_fuel = 0;
    for (auto &p: positions)
        total_fuel += abs(p - mid);

    cout << "total fuel required: " << total_fuel << endl;
}

void part_two(vector<int> &positions, int max_step) {
    int mid = positions[positions.size() / 2];
    uint64_t min_total_fuel = UINT_MAX;
    uint64_t total_fuel_above = 0;
    uint64_t total_fuel_below = 0;
    int position_above = 0;
    int position_below = 0;

    for (int step = 0; step <= max_step; step++) {
        total_fuel_above = 0;
        total_fuel_below = 0;
        position_above = mid + step;
        position_below = mid - step;

        for (auto &p : positions) {
            total_fuel_above += summation(abs(p - position_above));
            total_fuel_below += summation(abs(p - position_below));
        }

        if (total_fuel_above < min_total_fuel)
            min_total_fuel = total_fuel_above;
        if (total_fuel_below < min_total_fuel)
            min_total_fuel = total_fuel_below;
    }

    cout << "total fuel required: " << min_total_fuel << endl;
}

int main(int argc, char **argv) {
    vector<int> positions;
    int p;

    while(cin >> p) {
        if (cin.peek() == ',')
            cin.ignore();
        positions.push_back(p);
    }

    sort(positions.begin(), positions.end());

    if (argc == 2)
        part_two(positions, atoi(argv[1]));
    else
        part_one(positions);

    return 0;
}

