#include "..\common.c"
#include "..\lex.c"

char *PATTERN = "XMAS";

typedef enum { 
	TOP,
	TOP_RIGHT,
	RIGHT,
	BOTTOM_RIGHT,
	BOTTOM,
	BOTTOM_LEFT,
	LEFT,
	TOP_LEFT,
} Direction;

bool is_match(BUF(char **grid), int row, int col, Direction dir) {
	int pattern_i = 0;
	int pattern_len = (int)strlen(PATTERN);

	int x_inc, y_inc;
	switch (dir) {
		case TOP:          x_inc =  0; y_inc = -1; break;
		case TOP_RIGHT:    x_inc =  1; y_inc = -1; break;
		case RIGHT:        x_inc =  1; y_inc =  0; break;
		case BOTTOM_RIGHT: x_inc =  1; y_inc =  1; break;
		case BOTTOM:       x_inc =  0; y_inc =  1; break;
		case BOTTOM_LEFT:  x_inc = -1; y_inc =  1; break;
		case LEFT:         x_inc = -1; y_inc =  0; break;
		case TOP_LEFT:     x_inc = -1; y_inc = -1; break;
		default:
			assert(0);
			break;
	}

	for (int j=row, i=col;
		j >= 0 && j < buf_len(grid) && i >= 0 && i < buf_len(grid[j]) && pattern_i < pattern_len;
		j += y_inc, i += x_inc, ++pattern_i)
	{
		if (grid[j][i] != PATTERN[pattern_i]) break;
	}

	return pattern_i == pattern_len;
}

void part_one(BUF(char **grid)) {
	int count = 0;
	for (int j=0; j<buf_len(grid); ++j) {
		BUF(char *row) = grid[j];
		for (int i=0; i<buf_len(row); ++i) {
			int pattern_i = 0;
			if (is_match(grid, j, i, TOP))          count += 1;
			if (is_match(grid, j, i, TOP_RIGHT))    count += 1;
			if (is_match(grid, j, i, RIGHT))        count += 1;
			if (is_match(grid, j, i, BOTTOM_RIGHT)) count += 1;
			if (is_match(grid, j, i, BOTTOM))       count += 1;
			if (is_match(grid, j, i, BOTTOM_LEFT))  count += 1;
			if (is_match(grid, j, i, LEFT))         count += 1;
			if (is_match(grid, j, i, TOP_LEFT))     count += 1;
		}
	}
	printf("part one: Found %d matches of \"%s\"\n", count, PATTERN);
}

void part_two(BUF(char **grid)) {
	int count = 0;
	for (int j=1; j < buf_len(grid) - 1; ++j) {
		for (int i=1; i < buf_len(grid) - 1; ++i) {
			if (grid[j][i] == 'A') {
				char tl = grid[j-1][i-1];
				char tr = grid[j-1][i+1];
				char br = grid[j+1][i+1];
				char bl = grid[j+1][i-1];
				if (((tl == 'M' && br == 'S') || (tl == 'S' && br == 'M')) &&
					((tr == 'M' && bl == 'S') || (tr == 'S' && bl == 'M')))
				{
					count += 1;
				}
			}
		}
	}
	printf("part two: Found %d mas x's\n", count);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [input_filepath]\n", argv[0]);
		exit(1);
	}

	char *file_name = argv[1];
	
	char *file_data;
	U64 file_size;
	if (!read_entire_file(file_name, &file_data, &file_size)) {
		fatal("failed to read file %s", file_name);
	}

	BUF(char **grid) = NULL;
	char *cursor = file_data;
	while (*cursor != 0) {
		BUF(char *row) = NULL;
		while (*cursor != '\n') {
			if (isalpha((*cursor))) {
				buf_push(row, *cursor);
			}
			++cursor;
		}
		if (*cursor == '\n') {
			buf_push(grid, row);
		}
		++cursor;
	}

	printf("grid %dx%d\n", buf_len(grid), buf_len(grid[0]));

	part_one(grid);
	part_two(grid);

	return 0;
}

