#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

#define GOLD     0xFF01BAE8
#define GREEN    0xFF009701
#define RED      0xFF0101FF
#define DARK_RED 0xFF00007C
#define TEAL     0xFFE4DF78
#define MAGENTA  0xFF8F47D5

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

typedef struct {
	int x, y, w, h;
	U32 color;
	bool enabled;
} Cell;

typedef struct {
	BUF(Cell **cells);
	int row;
} Display;

U32 colors[] = {
	['X'] = GOLD,
	['M'] = GREEN,
	['A'] = RED,
	['S'] = DARK_RED,
};

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
		for (int i=0; i<buf_len(grid[j]); ++i) {
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

bool is_mas_x(BUF(char **grid), int row, int col) {
	if (row == 0 || col == 0 || row == buf_len(grid)-1 || col == buf_len(grid[row])-1) {
		return false;
	}

	if (grid[row][col] == 'A') {
		char tl = grid[row-1][col-1];
		char tr = grid[row-1][col+1];
		char br = grid[row+1][col+1];
		char bl = grid[row+1][col-1];
		if (((tl == 'M' && br == 'S') || (tl == 'S' && br == 'M')) &&
			((tr == 'M' && bl == 'S') || (tr == 'S' && bl == 'M')))
		{
			return true;
		}
	}

	return false;
}

void draw_cells(Display *display) {
	BUF(Cell **cells) = display->cells;

	int cell_h = (int)SCREEN_HEIGHT / (int)buf_len(cells);
	int cell_w = (int)SCREEN_WIDTH / (int)buf_len(cells[0]);
	int off_x = (int)((SCREEN_WIDTH - (float)(buf_len(cells[0]) * cell_w)) / 2.0f);
	int off_y = (int)((SCREEN_HEIGHT - (float)(buf_len(cells) * cell_h)) / 2.0f);

	for (int j=0; j<buf_len(cells); ++j) {
		for (int i=0; i<buf_len(cells[j]); ++i) {
			Cell cell = cells[j][i];
			if (cell.enabled) {
				drawbox(cell.x + off_x, cell.y + off_y, cell.w, cell.h, cell.color);
			}
		}
	}

	// draw scanline
	int scale = 8;
	int r = display->row;
	for (int i=0; i<buf_len(cells[r]); ++i) {
		Cell cell = cells[r][i];
		int x = cell.x - (scale/2) + off_x;
		int y = cell.y - (scale/2) + off_y;
		drawbox(x, y, cell.w + scale, cell.h + scale, cell.color);
	}
}

void highlight_x(BUF(Cell **cells), int row, int col, U32 color) {
	cells[row][col].color = MAGENTA;
	cells[row-1][col-1].color = MAGENTA;
	cells[row-1][col+1].color = MAGENTA;
	cells[row+1][col-1].color = MAGENTA;
	cells[row+1][col+1].color = MAGENTA;
}

void highlight_pattern(BUF(Cell **cells), int row, int col, Direction dir, U32 color) {
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

	for (int j=row, i=col, pattern_i=0;
		j >= 0 && j < buf_len(cells) && i >= 0 && i < buf_len(cells[j]) && pattern_i < pattern_len;
		j += y_inc, i += x_inc, ++pattern_i)
	{
		cells[j][i].color = TEAL;
	}
}

void visualize(BUF(char **grid)) {
	Display display = {0};

	// populate cells
	int cell_h = (int)SCREEN_HEIGHT / (int)buf_len(grid);
	int cell_w = (int)SCREEN_WIDTH / (int)buf_len(grid[0]);
	for (int j=0; j<buf_len(grid); ++j) {
		BUF (Cell *cell_row) = NULL;
		for (int i=0; i<buf_len(grid[j]); ++i) {
			Cell cell = {0};
			cell.x = i * cell_w;
			cell.y = j * cell_h;
			cell.w = cell_w;
			cell.h = cell_h;
			cell.color = colors[grid[j][i]];
			buf_push(cell_row, cell);
		}
		buf_push(display.cells, cell_row);
	}

	setupgif(0, 2, "ceres.gif");

	// first pass, reveal cells
	display.row = 0;
	for (int j=0; j<buf_len(grid); ++j) {
		clear();
		draw_cells(&display);
		nextframe();

		for (int i=0; i<buf_len(grid[j]); ++i) {
			display.cells[j][i].enabled = true;
		}
		display.row += 1;
	}

	// second pass, "xmas"
	display.row = 0;
	for (int j=0; j<buf_len(grid); ++j) {
		clear();
		draw_cells(&display);
		nextframe();

		for (int i=0; i<buf_len(grid[j]); ++i) {
			if (is_match(grid, j, i, TOP))
				highlight_pattern(display.cells, j, i, TOP, TEAL);
			if (is_match(grid, j, i, TOP_RIGHT))
				highlight_pattern(display.cells, j, i, TOP_RIGHT, TEAL);
			if (is_match(grid, j, i, RIGHT))
				highlight_pattern(display.cells, j, i, RIGHT, TEAL);
			if (is_match(grid, j, i, BOTTOM_RIGHT))
				highlight_pattern(display.cells, j, i, BOTTOM_RIGHT, TEAL);
			if (is_match(grid, j, i, BOTTOM))
				highlight_pattern(display.cells, j, i, BOTTOM, TEAL);
			if (is_match(grid, j, i, BOTTOM_LEFT))
				highlight_pattern(display.cells, j, i, BOTTOM_LEFT, TEAL);
			if (is_match(grid, j, i, LEFT))
				highlight_pattern(display.cells, j, i, LEFT, TEAL);
			if (is_match(grid, j, i, TOP_LEFT))
				highlight_pattern(display.cells, j, i, TOP_LEFT, TEAL);
		}
		display.row += 1;

	}

	// third pass, "mas" crosses
	display.row = 0;
	for (int j=0; j<buf_len(grid); ++j) {
		clear();
		draw_cells(&display);
		nextframe();

		for (int i=0; i<buf_len(grid[j]); ++i) {
			if (is_mas_x(grid, j, i)) {
				highlight_x(display.cells, j, i, MAGENTA);
			}
		}
		display.row += 1;
	}

	endgif();
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

	visualize(grid);

	return 0;
}

