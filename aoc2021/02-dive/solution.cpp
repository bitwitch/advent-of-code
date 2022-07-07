#include <iostream>

using namespace std;

void part_one() {
    int horizontal = 0;
    int depth = 0;

    string dir;
    int dist;
    while ( cin >> dir && cin >> dist ) {
        if (!dir.compare("forward"))
            horizontal += dist;
        else if (!dir.compare("down"))
            depth += dist;
        else if (!dir.compare("up"))
            depth -= dist;
    }

    cout << "result: " << horizontal*depth << endl;

}

void part_two() {
    int aim = 0;
    int horizontal = 0;
    int depth = 0;

    string command;
    int value;
    while ( cin >> command && cin >> value ) {
        if (!command.compare("up")) {
            aim -= value;
        } else if (!command.compare("down")) {
            aim += value;
        } else if (!command.compare("forward")) {
            horizontal += value;
            depth += aim * value;
        }
    }

    cout << "result: " << horizontal*depth << endl;
}

int main(int argc, char **argv) {
    if (argc == 2)
        part_two();
    else
        part_one();
    return 0;
}
