#include "..\common.c"
#include "..\..\base\base_inc.h"
#include "..\..\base\base_inc.c"

#define SCREEN_SIZE 1024
#define RED         0x29299e
#define WHITE       0xffffff

typedef struct {
	BUF(char *data);
	int rows;
	int cols;
} Grid;

int count_neighbors(Grid grid, int row, int col) {
	int count = 0;

	for (int row_off = -1; row_off <= 1; ++row_off) {
		for (int col_off = -1; col_off <= 1; ++col_off) {
			int j = row + row_off;
			int i = col + col_off;
			if (i >= 0 && i < grid.cols && j >= 0 && j < grid.rows && !(i == col && j == row)) {
				if (grid.data[j * grid.cols + i] == '@') {
					count += 1;
				}
			}
		}
	}

	return count;
}

void part_one(Grid grid) {
	int result = 0;

	for (int j=0; j<grid.rows; ++j) {
		for (int i=0; i<grid.cols; ++i) {
			char c = grid.data[j * grid.cols + i];
			if (c == '@') {
				int count = count_neighbors(grid, j, i);
				if (count < 4) result += 1;
			}
		}
	}

	printf("part one: %d\n", result);
}

int remove_accessible_rolls(Grid grid) {
	int result = 0;

	for (int j=0; j<grid.rows; ++j) {
		for (int i=0; i<grid.cols; ++i) {
			char c = grid.data[j * grid.cols + i];
			if (c == '@') {
				int count = count_neighbors(grid, j, i);
				if (count < 4)  {
					result += 1;
					grid.data[j * grid.cols + i] = '.';
				}
			}
		}
	}

	return result;
}

void part_two(Grid grid) {
	int result = 0;

	for (;;) {
		int count = remove_accessible_rolls(grid);
		result += count;
		if (count == 0) break;
	}

	printf("part two: %d\n", result);
}

void draw_grid(Grid grid, int cell_size) {
	int pad = (SCREEN_SIZE - grid.rows * cell_size) / 2;
	for (int j=0; j<grid.rows; ++j) {
		for (int i=0; i<grid.cols; ++i) {
			U32 color = grid.data[j * grid.cols + i] == '@' ? WHITE : RED;
			int x = pad + i * cell_size;
			int y = pad + j * cell_size;
			drawrect(x, y, x+cell_size, y+cell_size, color);
		}
	}
}

void visualize(Grid grid) {
	setupgif(1, 1, "printing.gif");

	int cell_size = MIN(SCREEN_SIZE / grid.rows, SCREEN_SIZE / grid.cols);
	draw_grid(grid, cell_size);
	nextframe();

	int skip_frames = 12;
	int frames = 0;

	int result = 0;

	for (;;) {

		int count = 0;
		for (int j=0; j<grid.rows; ++j) {
			for (int i=0; i<grid.cols; ++i) {
				char c = grid.data[j * grid.cols + i];
				if (c == '@') {
					int neighbors = count_neighbors(grid, j, i);
					if (neighbors < 4)  {
						count += 1;
						grid.data[j * grid.cols + i] = '.';

						draw_grid(grid, cell_size);
						if (frames++ % skip_frames == 0)
							nextframe();
					}
				}
			}
		}
		result += count;
		if (count == 0) break;
	}

	for (int i=0; i<16; ++i) nextframe();
	endgif();
}


int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [input_filepath]\n", argv[0]);
		exit(1);
	}

	char *filename = argv[1];
	
	char *file_data;
	U64 file_size;
	if (!read_entire_file(filename, &file_data, &file_size)) {
		fatal("failed to read file %s", filename);
	}

	Grid grid = {0};
	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == 0) break;
		++grid.rows;
		grid.cols = 0;
		while (*line != '\r' && *line != 0) {
			buf_push(grid.data, *line);
			++grid.cols;
			++line;
		}
	}

	Grid grid_copy = {0};
	grid_copy.rows = grid.rows;
	grid_copy.cols = grid.cols;
	for (int i=0; i<buf_len(grid.data); ++i) {
		buf_push(grid_copy.data, grid.data[i]);
	}

	part_one(grid);
	part_two(grid);
	visualize(grid_copy);

	return 0;
}
