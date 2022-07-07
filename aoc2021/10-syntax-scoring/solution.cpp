#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <stack>
#include <vector>

using namespace std;

unordered_map<char, int> syntax_score_table = {
    { ')', 3 },
    { ']', 57 },
    { '}', 1197 },
    { '>', 25137 }
};

unordered_map<char, uint64_t> autocomplete_score_table = {
    { ')', 1 },
    { ']', 2 },
    { '}', 3 },
    { '>', 4 }
};

unordered_map<char, char> matching_brace = {
    { ')', '(' },
    { ']', '[' },
    { '}', '{' },
    { '>', '<' },
    { '(', ')' },
    { '[', ']' },
    { '{', '}' },
    { '<', '>' }
};

bool is_corrupted(string &line, char *first_illegal_character) {
    stack<char> braces;
    for (auto c : line) {
        if (c == '(' || c == '[' || c == '{' || c == '<')
            braces.push(c);
        else if (c == ')' || c == ']' || c == '}' || c == '>') {
            if (braces.top() == matching_brace[c]) {
                braces.pop();
            } else {
                *first_illegal_character = c;
                return true;
            }
        }
    }
    return false;
}

bool is_incomplete(string &line, string *closing_sequence) {
    stack<char> braces;
    for (auto c : line) {
        if (c == '(' || c == '[' || c == '{' || c == '<')
            braces.push(c);
        else if (c == ')' || c == ']' || c == '}' || c == '>') {
            if (braces.top() == matching_brace[c]) {
                braces.pop();
            } else {
                return false;
            }
        }
    }

    if (braces.size() == 0)
        return false;

    while (braces.size() > 0) {
        closing_sequence->push_back(matching_brace[braces.top()]);
        braces.pop();
    }

    return true;
}


void part_one(vector<string> &lines) {
    int score = 0;
    for (auto &line : lines) {
        char c;
        if (is_corrupted(line, &c))
            score += syntax_score_table[c];
    }
    cout << "score: " << score << endl;
}


void part_two(vector<string> &lines) {
    vector<uint64_t> scores;
    for (auto &line : lines) {
        uint64_t score = 0;
        string closing_sequence;
        if (is_incomplete(line, &closing_sequence)) {
            for (auto &c : closing_sequence)
                score = score * 5 + autocomplete_score_table[c];
            scores.push_back(score);
        }
    }
    sort(scores.begin(), scores.end());
    int mid = scores.size() / 2;
    cout << "score: " << scores[mid] << endl;
}


int main(int argc, char **argv) {
    vector<string> lines;
    string line;
    while(getline(cin, line))
        lines.push_back(line);

    if (argc == 2)
        part_two(lines);
    else 
        part_one(lines);

    return 0;
}
