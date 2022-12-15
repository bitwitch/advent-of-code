#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

typedef enum {
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST
} Direction;

bool visible_from_direction(int *grid, int rows, int cols, int row, int col, Direction direction) {
	int i, j, w = cols;
	int height = grid[row*cols+col];

	bool visible = true;

	switch (direction) {
	case DIRECTION_NORTH:
		i = col;
		for (j=row-1; j>=0; --j)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	case DIRECTION_EAST:
		j = row;
		for (i=col+1; i<cols; ++i)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	case DIRECTION_SOUTH:
		i = col;
		for (j=row+1; j<rows; ++j)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	case DIRECTION_WEST:
		j = row;
		for (i=col-1; i>=0; --i)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	default:
		fprintf(stderr, "visible_from_direction: unknown direction %d\n", direction);
		exit(1);
		break;
	}

	return visible;
}

int view_distance_in_direction(int *grid, int rows, int cols, int row, int col, Direction direction) {
	int i, j, w = cols;
	int height = grid[row*cols+col];

	int distance = 0;

	switch (direction) {
	case DIRECTION_NORTH:
		i = col;
		for (j=row-1; j>=0; --j) {
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	case DIRECTION_EAST:
		j = row;
		for (i=col+1; i<cols; ++i) {
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	case DIRECTION_SOUTH:
		i = col;
		for (j=row+1; j<rows; ++j) {
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	case DIRECTION_WEST:
		j = row;
		for (i=col-1; i>=0; --i) { 
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	default:
		fprintf(stderr, "visible_from_direction: unknown direction %d\n", direction);
		exit(1);
		break;
	}

	return distance;
}


void part_one(int *grid, int rows, int cols) {
	int i, j, visible = 0;
	for (j=1; j<rows-1; ++j) {
		for (i=1; i<cols-1; ++i) {
			if (
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_NORTH) ||
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_EAST) ||
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_SOUTH) ||
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_WEST)
			){
				++visible;
			}
		}
	}

	visible += 2*cols + 2*(rows-2);

	printf("part_one: %d\n", visible);
}

void part_two(int *grid, int rows, int cols) {
	int i, j, score = 0, best_score = 0;
	for (j=0; j<rows; ++j) {
		for (i=0; i<cols; ++i) {
			score =
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_NORTH) *
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_EAST)  *
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_SOUTH) * 
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_WEST);
			best_score = Max(score, best_score);
		}
	}
	printf("part two: %d\n", best_score);
}

int *build_grid(U8 *input, int *out_rows, int *out_cols) {
	int *grid = NULL;
	char *line;
	U64 cols, rows = 0;
	do {
		cols = 0;
		// parse each character in line as int and put in grid
		for (line = arb_chop_by_delimiter((char **)&input, "\n"); *line != '\0'; ++line) {
			arrput(grid, *line - 48);
			++cols;
		}
		*out_cols = cols;
		++rows;
	} while (*input != '\0');
	*out_rows = rows;
	return grid;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Must supply filepath to puzzle input\n");
		exit(1);
	}

	FILE *fp;
	U8 *file_data;
	size_t file_size;
	int rows, cols, *grid;

	fp = fopen(argv[1], "r");
	if (!fp) { perror("fopen"); exit(1); }

	int ok = arb_read_entire_file(fp, &file_data, &file_size);
	if (ok != ARB_READ_ENTIRE_FILE_OK) {
		fprintf(stderr, "Error: arb_read_entire_file()\n");
		fclose(fp);
		exit(1);
	}

	fclose(fp);

	grid = build_grid(file_data, &rows, &cols);

	part_one(grid, rows, cols);
	part_two(grid, rows, cols);

	return 0;
}
