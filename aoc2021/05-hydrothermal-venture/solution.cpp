#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#define MIN(a,b) ((a < b) ? (a) : (b)) 
#define MAX(a,b) ((a > b) ? (a) : (b))

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        if (x == other.x && y == other.y)
            return true;
        return false;
    }
};

struct Line {
    Point p1, p2;
    bool operator==(const Line& other) const {
        if (p1 == other.p1 && p2 == other.p2)
            return true;
        return false;
    }
    void print() {
        std::cout << this << " (" << p1.x << ", " << p1.y << ") -> (" << p2.x << ", " << p2.y << ")" << std::endl;
    }
};

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//  SLOW BUT SIMPLE SOLUTION
//////////////////////////////////////////////////////////////////////////////
void get_grid_dimensions(vector<Line> &lines, int *min_x, int *max_x, int *min_y, int *max_y) {
    *min_x = INT_MAX; *min_y = INT_MAX;
    *max_x = INT_MIN; *max_y = INT_MIN;
    for (auto& line : lines) {
        if (line.p1.x < *min_x) *min_x = line.p1.x;
        if (line.p1.x > *max_x) *max_x = line.p1.x;
        if (line.p2.x < *min_x) *min_x = line.p2.x;
        if (line.p2.x > *max_x) *max_x = line.p2.x;
        if (line.p1.y < *min_y) *min_y = line.p1.y;
        if (line.p1.y > *max_y) *max_y = line.p1.y;
        if (line.p2.y < *min_y) *min_y = line.p2.y;
        if (line.p2.y > *max_y) *max_y = line.p2.y;
    }
}


bool point_in_segment(int x, int y, Line line) {
    int x1 = MIN(line.p1.x, line.p2.x);
    int x2 = MAX(line.p1.x, line.p2.x);
    int y1 = MIN(line.p1.y, line.p2.y);
    int y2 = MAX(line.p1.y, line.p2.y);

    // horizontal
    if (y1 == y2)
        return y == y1 && x >= x1 && x <= x2;
    // vertical
    else if (line.p1.x == line.p2.x)
        return x == x1 && y >= y1 && y <= y2;
    // diagonal
    else {
        int left_y = line.p1.x < line.p2.x ? line.p1.y : line.p2.y;
        int right_y = line.p1.x > line.p2.x ? line.p1.y : line.p2.y;
        int m = right_y > left_y ? 1 : -1;
        int b = line.p1.y - m * line.p1.x;
        return y == m*x+b && x >= x1 && x <= x2;
    }
}

void solution_slow_but_simple(vector<Line> &lines) {
    int result = 0;

    int min_x, max_x, min_y, max_y;
    get_grid_dimensions(lines, &min_x, &max_x, &min_y, &max_y);

    printf("min_x=%d max_x=%d  min_y=%d max_y=%d\n", min_x, max_x, min_y, max_y);

    for (int j=min_y; j<=max_y; ++j)
    for (int i=min_x; i<=max_x; ++i) {
        int count = 0;
        for (auto& line : lines)
            if (point_in_segment(i, j, line)) ++count;
        if (count > 1) ++result;
    }

    printf("Found %d overlapping points\n", result);
}

//////////////////////////////////////////////////////////////////////////////
//  MORE COMPLICATED BUT FASTER
//////////////////////////////////////////////////////////////////////////////

namespace std {
    template<> struct hash<Point> {
        uint64_t operator()(const Point& p) const noexcept {
            return ((uint64_t)p.x)<<32 | (uint64_t)p.y;
        }
    };
}


struct Range {
    int min, max; // both inclusive
};


void part_one_fast(vector<Line> &lines) {
    unordered_set<Point> points;
    unordered_map<int, vector<Range>> horizontal_lines;
    unordered_map<int, vector<Range>> vertical_lines;

    // generate a list of horizontal and vertical lines
    for (auto &line : lines) {
        if (line.p1.x == line.p2.x) {
            if (vertical_lines.count(line.p1.x) == 0)
                vertical_lines.insert({line.p1.x, vector<Range>()});

            Range range;
            range.min = min(line.p1.y, line.p2.y);
            range.max = max(line.p1.y, line.p2.y);

            vertical_lines[line.p1.x].push_back(range);
        }

        else if (line.p1.y == line.p2.y) {
            if (horizontal_lines.count(line.p1.y) == 0)
                horizontal_lines.insert({line.p1.y, vector<Range>()});

            Range range;
            range.min = min(line.p1.x, line.p2.x);
            range.max = max(line.p1.x, line.p2.x);

            horizontal_lines[line.p1.y].push_back(range);
        }
    }


    // for each horizontal line, check against all vertical lines, 
    // then check against horizontal lines at the same y
    for (auto &horiz : horizontal_lines) {
        int y = horiz.first;
        vector<Range> &x_ranges = horiz.second;

        for (auto &vert : vertical_lines) {
            int x = vert.first;
            vector<Range> &y_ranges = vert.second;
            for (auto &y_range : y_ranges) {
                if (y <= y_range.max && y >= y_range.min) {
                    for (auto &x_range : x_ranges) {
                        if (x <= x_range.max && x >= x_range.min) {
                            Point p = {x, y};
                            points.insert(p);
                        }
                    }
                }
            }
        }

        // check against horizontal lines at the same y
        for (auto &x_range : x_ranges) {
            for (auto &other_x_range : x_ranges) {
                if (&other_x_range == &x_range) continue;

                int min, max;
                if (x_range.min >= other_x_range.min && x_range.min <= other_x_range.max) {
                    min = x_range.min;
                } else if (other_x_range.min >= x_range.min && other_x_range.min <= x_range.max) {
                    min = other_x_range.min;
                } else {
                    continue;
                }

                if (x_range.max >= other_x_range.min && x_range.max <= other_x_range.max) {
                    max = x_range.max;
                } else {
                    max = other_x_range.max;
                }

                for (int i=min; i<=max; i++) {
                    Point p = {i, y};
                    points.insert(p);
                }
            }
        }
    }

    // check against vertical lines at the same x
    for (auto &vert : vertical_lines) {
        int x = vert.first;
        vector<Range> &y_ranges = vert.second;

        for (auto &y_range : y_ranges) {
            for (auto &other_y_range : y_ranges) {
                if (&other_y_range == &y_range) continue;

                int min, max;
                if (y_range.min >= other_y_range.min && y_range.min <= other_y_range.max) {
                    min = y_range.min;
                } else if (other_y_range.min >= y_range.min && other_y_range.min <= y_range.max) {
                    min = other_y_range.min;
                } else {
                    continue;
                }

                if (y_range.max >= other_y_range.min && y_range.max <= other_y_range.max) {
                    max = y_range.max;
                } else {
                    max = other_y_range.max;
                }

                for (int i=min; i<=max; i++) {
                    Point p = {x, i};
                    points.insert(p);
                }
            }
        }
    }


    cout << "points: " << points.size() << endl;
}




enum Orientation { COLLINEAR, CLOCKWISE, COUNTER_CLOCKWISE };

Orientation get_orientation(Point a, Point b, Point c) {
    float slope_comparison = (float)(b.y - a.y) * (float)(c.x - b.x) - (float)(c.y - b.y) * (float)(b.x - a.x);

    if (slope_comparison == 0)
        return COLLINEAR;

    if (slope_comparison > 0)
        return CLOCKWISE;

    return COUNTER_CLOCKWISE;
}


// incomplete, i started this years ago then came back and did the slow but simple version quickly
// and got the solution. i don't feel like fixing this up
void part_two(vector<Line> &lines) {
    unordered_set<Point> points;

    for (auto &line : lines) {
        for (auto &other : lines) {

            if (&line == &other) continue;

            Orientation o1 = get_orientation(line.p1, line.p2, other.p1);
            Orientation o2 = get_orientation(line.p1, line.p2, other.p2);
            Orientation o3 = get_orientation(other.p1, other.p2, line.p1);
            Orientation o4 = get_orientation(other.p1, other.p2, line.p2);

            // non-collinear intersection
            if (o1 != o2 && o3 != o4) {
                int x_intersect, y_intersect;
                if (line.p1.x == line.p2.x) {
                    x_intersect = line.p1.x;
                    float m2 = (float)(other.p2.y - other.p1.y) / (float)(other.p2.x - other.p1.x);
                    float b2 = (float)(other.p1.y) - (m2 * (float)other.p1.x);
                    y_intersect = (int)(m2 * (float)x_intersect + b2);

                } else if (other.p1.x == other.p2.x) {
                    x_intersect = other.p1.x;
                    float m1 = (float)(line.p2.y - line.p1.y) / (float)(line.p2.x - line.p1.x);
                    float b1 = (float)(line.p1.y) - (m1 * (float)line.p1.x);
                    y_intersect = (int)(m1 * (float)x_intersect + b1);

                } else {
                    float m1 = (float)(line.p2.y - line.p1.y) / (float)(line.p2.x - line.p1.x);
                    float b1 = (float)(line.p1.y) - (m1 * (float)line.p1.x);

                    float m2 = (float)(other.p2.y - other.p1.y) / (float)(other.p2.x - other.p1.x);
                    float b2 = (float)(other.p1.y) - (m2 * (float)other.p1.x);

                    x_intersect = (int)((b2 - b1) / (m1 - m2));
                    y_intersect = (int)(m1 * (float)x_intersect + b1);
                }

                Point p = {x_intersect, y_intersect};
                points.insert(p);
            }

            // collinear intersection
            else if (o1 == COLLINEAR && o2 == COLLINEAR) {
                int line_min_x = min(line.p1.x, line.p2.x);
                int line_max_x = max(line.p1.x, line.p2.x);
                int line_min_y = min(line.p1.y, line.p2.y);
                int line_max_y = max(line.p1.y, line.p2.y);

                int other_min_x = min(other.p1.x, other.p2.x);
                int other_max_x = max(other.p1.x, other.p2.x);
                int other_min_y = min(other.p1.y, other.p2.y);
                int other_max_y = max(other.p1.y, other.p2.y);

                if ((line_min_x >= other_min_x && line_min_x <= other_max_x &&
                    line_min_y >= other_min_y && line_min_y <= other_max_y) 
                    || 
                    (line_max_x >= other_min_x && line_max_x <= other_max_x &&
                    line_max_y >= other_min_y && line_max_y <= other_max_y))
                {
                    // vertical
                    if (line.p1.x == line.p2.x) {
                        for (int y = max(line_min_y, other_min_y); y <= min(line_max_y, other_max_y); y++) {
                            Point p = {line.p1.x, y};
                            points.insert(p);
                        }

                    // horizontal
                    } else if (line.p1.y == line.p2.y) {
                        for (int x = max(line_min_x, other_min_x); x <= min(line_max_x, other_max_x); x++) {
                            Point p = {x, line.p1.y};
                            points.insert(p);
                        }

                    // diagonal 
                    } else {
                        float m1 = (float)(line.p2.y - line.p1.y) / (float)(line.p2.x - line.p1.x);
                        float b1 = (float)(line.p1.y) - (m1 * (float)line.p1.x);
                        for (int x = max(line_min_x, other_min_x); x <= min(line_max_x, other_max_x); x++) {
                            int y = (int)(m1 * (float)x + b1); 
                            Point p = {x, y};
                            points.insert(p);
                        }
                    }
                }
            }
        }
    }


    cout << "points: " << points.size() << endl;
}

int main(int argc, char **argv) {

    vector<Line> lines;

    string input_line;
    while(getline(cin, input_line)) {
        Line line;
        stringstream ss(input_line);

        ss >> line.p1.x;
        ss.ignore(); // ignore comma
        ss >> line.p1.y;
        ss.ignore(); ss.ignore(); ss.ignore(); ss.ignore(); // ignore spaces and two character arrow 
        ss >> line.p2.x;
        ss.ignore(); // ignore comma
        ss >> line.p2.y;

        lines.push_back(line);
    }

    printf("There are %lu lines\n", lines.size());

    if (argc == 2)
        //part_one_fast(lines);
        part_two(lines);
    else
        solution_slow_but_simple(lines);

    return 0;
}




