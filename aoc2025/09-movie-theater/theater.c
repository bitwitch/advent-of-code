#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE 1024
#define RED         0x1212f4
#define GREEN       0x12f412

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	int x0, x1;
	int y0, y1;
} Bounds;

typedef struct {
	BUF(Vec2 *tiles);
	Bounds bounds;
	int cell_size;
} Grid;

void part_one(Grid grid) {
	S64 max_area = 0;
	for (int i=0; i<buf_len(grid.tiles)-1; ++i) {
		for (int j=0; j<buf_len(grid.tiles); ++j) {
			S64 w = llabs(grid.tiles[i].x - grid.tiles[j].x) + 1;
			S64 h = llabs(grid.tiles[i].y - grid.tiles[j].y) + 1;
			S64 area = w*h;
			if (area > max_area) max_area = area;
		}
	}
		
	printf("part one: %lld\n", max_area);
}

Vec2 map_to_screen(Vec2 pos, Bounds bounds) {
	F32 tx = (F32)(pos.x - bounds.x0) / (F32)(bounds.x1 - bounds.x0);
	F32 ty = (F32)(pos.y - bounds.y0) / (F32)(bounds.y1 - bounds.y0);
	return (Vec2){
		.x = (int)(SCREEN_SIZE * tx),
		.y = (int)(SCREEN_SIZE * ty),
	};
}

void draw_grid_part(Grid grid, Bounds bounds) {
	int cell_size = MAX(3, grid.cell_size);
	for (int i=0; i<buf_len(grid.tiles); ++i) {
		Vec2 tile = grid.tiles[i];
		Vec2 coords = map_to_screen(grid.tiles[i], grid.bounds);
		if (tile.x >= bounds.x0 && tile.x <= bounds.x1 && tile.y >= bounds.y0 && tile.y <= bounds.y1) {
			drawbox(coords.x, coords.y, cell_size, cell_size, GREEN);
		} else {
			drawbox(coords.x, coords.y, cell_size, cell_size, RED);
		}
	}
}

void draw_grid(Grid grid) {
	int cell_size = MAX(3, grid.cell_size);
	for (int i=0; i<buf_len(grid.tiles); ++i) {
		Vec2 coords = map_to_screen(grid.tiles[i], grid.bounds);
		drawbox(coords.x, coords.y, cell_size, cell_size, RED);
	}
}

bool inside_rect(Grid grid, int _a, int _b, int _check, bool inclusive) {
	Vec2 a = grid.tiles[_a];
	Vec2 b = grid.tiles[_b];
	Vec2 check = grid.tiles[_check];
	Bounds bounds = {
		.x0 = MIN(a.x, b.x),
		.x1 = MAX(a.x, b.x),
		.y0 = MIN(a.y, b.y),
		.y1 = MAX(a.y, b.y),
	};
	if (inclusive) {
		return check.x >= bounds.x0 && check.x <= bounds.x1 && 
			   check.y >= bounds.y0 && check.y <= bounds.y1;
	} else {
		return check.x > bounds.x0 && check.x < bounds.x1 && 
			   check.y > bounds.y0 && check.y < bounds.y1;
	}
}

bool different_edge(Grid grid, int _a, int _b, int _check1, int _check2) {
	Vec2 a = grid.tiles[_a];
	Vec2 b = grid.tiles[_b];
	Vec2 check1 = grid.tiles[_check1];
	Vec2 check2 = grid.tiles[_check2];
	bool v_edge = check1.x == a.x || check1.x == b.x;
	bool h_edge = check1.y == a.y || check1.y == b.y;

	if (v_edge && check1.x != check2.x) return true;
	if (h_edge && check1.y != check2.y) return true;

	return false;
}

bool tile_reduces_rect(Grid grid, int _a, int _b, int _check) {
	if (inside_rect(grid, _a, _b, _check, false)) {
		return true;
	}

	Vec2 a = grid.tiles[_a];
	Vec2 b = grid.tiles[_b];
	Vec2 check = grid.tiles[_check];
	Bounds bounds = {
		.x0 = MIN(a.x, b.x),
		.x1 = MAX(a.x, b.x),
		.y0 = MIN(a.y, b.y),
		.y1 = MAX(a.y, b.y),
	};


	// does this tile and a neighbor form a line that bisects the rect
	int _prev = _check == 0 ? buf_len(grid.tiles) - 1 : _check - 1;
	int _next = (_check == buf_len(grid.tiles) - 1) ? 0 : _check + 1;
	Vec2 prev = grid.tiles[_prev];
	Vec2 next = grid.tiles[_next];
	
	// check horizontal bisect
	if (check.y > bounds.y0 && check.y < bounds.y1 ) {
		// from left
		if (check.x <= bounds.x0) {
			if (_prev != _a && _prev != _b && prev.x >= bounds.x0) {
				return true;
			}
			if (_next != _a && _next != _b && next.x >= bounds.x0) {
				return true;
			}
		}
		// from right
		if (check.x >= bounds.x1) {
			if (_prev != _a && _prev != _b && prev.x <= bounds.x1) {
				return true;
			}
			if (_next != _a && _next != _b && next.x <= bounds.x1) {
				return true;
			}
		}
	}

	// check vertical bisect
	if (check.x > bounds.x0 && check.x < bounds.x1 ) {
		// from left
		if (check.y <= bounds.y0) {
			if (_prev != _a && _prev != _b && prev.y >= bounds.y0) {
				return true;
			}
			if (_next != _a && _next != _b && next.y >= bounds.y0) {
				return true;
			}
		}
		// from right
		if (check.y >= bounds.y1) {
			if (_prev != _a && _prev != _b && prev.y <= bounds.y1) {
				return true;
			}
			if (_next != _a && _next != _b && next.y <= bounds.y1) {
				return true;
			}
		}
	}

	return false;
}

void part_two(Grid grid) {
	S64 max_area = 0;
	for (int i=0; i<buf_len(grid.tiles)-1; ++i) {
		for (int j=0; j<buf_len(grid.tiles); ++j) {
			bool valid_rect = true;
			for (int k=0; k<buf_len(grid.tiles); ++k) {
				if (k != i && k != j && tile_reduces_rect(grid, i, j, k)) {
					valid_rect = false;
					break;
				}
			}
			if (valid_rect) {
				S64 w = llabs(grid.tiles[i].x - grid.tiles[j].x) + 1;
				S64 h = llabs(grid.tiles[i].y - grid.tiles[j].y) + 1;
				S64 area = w*h;
				if (area > max_area) max_area = area;
			}
		}
	}

	printf("part two: %lld\n", max_area);
}

void visualize(Grid grid) {
	setupgif(1, 1, "theater.gif");

	// trace a path from tile to tile
	draw_grid(grid);
	nextframe();
	nextframe();
	for (int i=0; i<buf_len(grid.tiles)-1; ++i) {
		Vec2 a = map_to_screen(grid.tiles[i], grid.bounds);
		Vec2 b = map_to_screen(grid.tiles[i+1], grid.bounds);
		drawline(a.x, a.y, b.x, b.y, 1, GREEN);
		draw_grid(grid);
		nextframe();
	}
	
	S64 max_area = 0;
	int max_a = 0;
	int max_b = 0;
	for (int i=0; i<buf_len(grid.tiles)-1; ++i) {
		for (int j=0; j<buf_len(grid.tiles); ++j) {
			bool valid_rect = true;
			for (int k=0; k<buf_len(grid.tiles); ++k) {
				if (k != i && k != j && tile_reduces_rect(grid, i, j, k)) {
					valid_rect = false;
					break;
				}
			}
			if (valid_rect) {
				S64 w = llabs(grid.tiles[i].x - grid.tiles[j].x) + 1;
				S64 h = llabs(grid.tiles[i].y - grid.tiles[j].y) + 1;
				S64 area = w*h;
				if (area > max_area) {
					max_area = area;
					max_a = i;
					max_b = j;
				}
			}
		}
	}

	Vec2 a = map_to_screen(grid.tiles[max_a], grid.bounds);
	Vec2 b = map_to_screen(grid.tiles[max_b], grid.bounds);
	drawrect(a.x, a.y, b.x, b.y, GREEN);
	for (int i=0; i<64; ++i) nextframe();

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
	grid.bounds.x0 = INT_MAX;
	grid.bounds.y0 = INT_MAX;
	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == '\r' || *line == 0) break;
		Vec2 t = {0};
		sscanf(line, "%d,%d", &t.x, &t.y);
		buf_push(grid.tiles, t);
		if (t.x > grid.bounds.x1) grid.bounds.x1 = t.x;
		if (t.y > grid.bounds.y1) grid.bounds.y1 = t.y;
		if (t.x < grid.bounds.x0) grid.bounds.x0 = t.x;
		if (t.y < grid.bounds.y0) grid.bounds.y0 = t.y;
	}


	grid.bounds.x0 = MAX(0, grid.bounds.x0 - 3);
	grid.bounds.y0 = MAX(0, grid.bounds.y0 - 3);
	grid.bounds.x1 += 3;
	grid.bounds.y1 += 3;

	int largest_dim = MIN(grid.bounds.x1 - grid.bounds.x0, grid.bounds.y1 - grid.bounds.y0);
	grid.cell_size = (int)(SCREEN_SIZE / largest_dim);

	part_one(grid);
	part_two(grid);
	visualize(grid);

	return 0;
}
