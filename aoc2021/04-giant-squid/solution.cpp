#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

struct Cell {
    int value;
    bool marked;
};

struct Board {
    Cell cells[5][5];
};

void mark(Board &board, int draw) {
    for (int j=0; j<5; j++) {
        for (int i=0; i<5; i++) {
            if (board.cells[j][i].value == draw) {
                board.cells[j][i].marked = true;
            }
        }
    }
}

bool check_bingo(Board &board) {
    bool bingo;

    // check rows
    for (int j=0; j<5; j++) {
        bingo = true;
        for (int i=0; i<5; i++) {
            if (!board.cells[j][i].marked) {
                bingo = false;
                break;
            }
        }
        if (bingo) return true;
    }

    // check cols
    for (int i=0; i<5; i++) {
        bingo = true;
        for (int j=0; j<5; j++) {
            if (!board.cells[j][i].marked) {
                bingo = false;
                break;
            }
        }
        if (bingo) return true;
    }

    return false;
}

int get_score(Board &board, int last_draw) {
    int score = 0;
    for (int j=0; j<5; j++) {
        for (int i=0; i<5; i++) {
            if (!board.cells[j][i].marked)
                score += board.cells[j][i].value;
        }
    }

    score *= last_draw;

    return score;
}

void part_one(vector<Board> &boards, vector<int> &draws) {
    for (auto draw : draws) {
        for (auto &board : boards) {
            mark(board, draw);
            if (check_bingo(board)) {
                cout << "winning score: " << get_score(board, draw) << endl;
                return;
            }
        }
    }
}

void part_two(vector<Board> &boards, vector<int> &draws) {
    unordered_set<int> non_winning_boards;
    for (int i=0; i<boards.size(); i++)
        non_winning_boards.insert(i);

    for (auto draw : draws) {
        for (int i=0; i<boards.size(); i++) {
            if (non_winning_boards.count(i) == 0) continue;
            mark(boards[i], draw);
            if (check_bingo(boards[i])) {
                if (non_winning_boards.size() == 1) {
                    cout << "last winning score: " << get_score(boards[i], draw) << endl;
                    return;
                }
                non_winning_boards.erase(i);
            }
        }
    }
}

int main(int argc, char **argv) {

    // parse number draws
    string line;
    vector<int> draws;
    getline(cin, line);
    stringstream ss(line);
    int n;
    while (ss >> n) {
        if (ss.peek() == ',')
            ss.ignore();
        cout << n << endl;
        draws.push_back(n);
    }

    // parse boards
    vector<Board> boards;

    while (!cin.eof()) {
        Board board;
        for (int j=0; j<5; j++) {
            for (int i=0; i<5; i++) {
                cin >> board.cells[j][i].value;
                board.cells[j][i].marked = false;
            }
        }
        boards.push_back(board);
    }

    if (argc == 2)
        part_two(boards, draws);
    else
        part_one(boards, draws);

    return 0;
}
