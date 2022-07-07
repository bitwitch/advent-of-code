#include <iostream>
#include <string>
#include <vector>

using namespace std;

void part_one(vector<int> &codes, int n_bits) {
    int gamma = 0;
    int epsilon = 0;
    for (int i=0; i<n_bits; i++) {
        int ones = 0;
        for (auto &code : codes) {
            int bit = (code >> i) & 1;
            ones += bit;
        }
        if (ones > codes.size() - ones)
            gamma |= 1 << i;
        else
            epsilon |= 1 << i;
    }

    cout << "power consumption: " << gamma*epsilon << endl;
}

void filter_candidates(vector<int> *candidates, int bit_position, int bit_value) {
    for (auto it = candidates->begin(); it != candidates->end(); ) {
        if (((*it >> bit_position) & 1) == bit_value) {
            // bit at bit_position matches bit_value
            it++;
        } else {
            it = candidates->erase(it);
        }
    }
}

void part_two(vector<int> &codes, int n_bits) {
    int oxygen_generator_rating, co2_scrubber_rating;
    vector<int> o2_candidates, co2_candidates;

    o2_candidates = codes;
    for (int i=n_bits-1; i>=0; i--) {
        int ones = 0;
        int most_common;
        for (auto code : o2_candidates) {
            int bit = (code >> i) & 1;
            ones += bit;
        }

        most_common = ones >= o2_candidates.size() - ones ? 1 : 0;

        if (o2_candidates.size() > 1)
            filter_candidates(&o2_candidates, i, most_common);

        if (o2_candidates.size() == 1) {
            oxygen_generator_rating = o2_candidates[0];
            break;
        }
    }

    co2_candidates = codes;
    for (int i=n_bits-1; i>=0; i--) {
        int ones = 0;
        int least_common;
        for (auto code : co2_candidates) {
            int bit = (code >> i) & 1;
            ones += bit;
        }

        least_common = ones >= co2_candidates.size() - ones ? 0 : 1;

        if (co2_candidates.size() > 1)
            filter_candidates(&co2_candidates, i, least_common);


        if (co2_candidates.size() == 1) {
            co2_scrubber_rating = co2_candidates[0];
            break;
        }
    }

    cout << "life support rating: " << oxygen_generator_rating * co2_scrubber_rating << endl;
}


int main(int argc, char **argv) {
    int n_bits = 0;
    vector<int> codes;
    string binary;
    while (cin >> binary) {
        codes.push_back(stoi(binary, 0, 2));
        n_bits = binary.size();
    }

    if (argc == 2)
        part_two(codes, n_bits);
    else
        part_one(codes, n_bits);
    return 0;
}
