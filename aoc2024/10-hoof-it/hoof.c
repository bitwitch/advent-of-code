#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE   1024
#define BLUE          0xFFFFEB00
#define DARK_BLUE     0xFFE3CA09
#define SKIP_FRAMES   30

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	int x, y;
	BUF(Vec2 *peaks);
	int trails;
} Trailhead;

typedef struct {
	BUF(int **grid);
	BUF(Trailhead *trailheads);
} Topography;

int cell_size = 0;
int pad = 0;
int frames = 0;

BUF(Vec2 **paths) = NULL;

U32 colors[] = {
	0xFF0C8524,
	0xFF4BBE8C,
	0xFF5FCDAF,
	0xFF73EBF5,
	0xFF61D4F7,
	0xFF50BEFA,
	0xFF2D96F0,
	0xFF2873C3,
	0xFF1E5596,
	0xFF04234A,
};

bool is_on_grid(BUF(int **grid), int x, int y) {
	return y >= 0 && y < buf_len(grid) &&		
		   x >= 0 && x < buf_len(grid[0]);
}

void hike(Trailhead *trailhead, BUF(int **grid), int x, int y) {
	if (grid[y][x] == 9) {
		++trailhead->trails;
		for (int i=0; i<buf_len(trailhead->peaks); ++i) {
			if (trailhead->peaks[i].x == x && trailhead->peaks[i].y == y) {
				return;
			}
		}
		buf_push(trailhead->peaks, (Vec2){x,y});
		return;
	}

	if (is_on_grid(grid, x+1, y) && grid[y][x+1] == grid[y][x] + 1) 
		hike(trailhead, grid, x+1, y);
	if (is_on_grid(grid, x-1, y) && grid[y][x-1] == grid[y][x] + 1) 
		hike(trailhead, grid, x-1, y);
	if (is_on_grid(grid, x, y+1) && grid[y+1][x] == grid[y][x] + 1) 
		hike(trailhead, grid, x, y+1);
	if (is_on_grid(grid, x, y-1) && grid[y-1][x] == grid[y][x] + 1) 
		hike(trailhead, grid, x, y-1);
}

void draw_grid(BUF(int **grid)) {
	for (int j=0; j<buf_len(grid); ++j) {
		for (int i=0; i<buf_len(grid); ++i) {
			int x = i * cell_size + pad;
			int y = j * cell_size + pad;
			int height = grid[j][i];
			drawbox(x, y, cell_size, cell_size, colors[height]);
		}
	}
}


void draw_path(BUF(Vec2 *path), U32 color) {
	for (int i=0; i<buf_len(path) - 1; ++i) {
		int x0 = path[ i ].x * cell_size + cell_size/2 + pad;
		int y0 = path[ i ].y * cell_size + cell_size/2 + pad;
		int x1 = path[i+1].x * cell_size + cell_size/2 + pad;
		int y1 = path[i+1].y * cell_size + cell_size/2 + pad;
		drawline(x0, y0, x1, y1, 5, color);
	}
}

void draw_paths(BUF(Vec2 **paths)) {
	for (int i=0; i<buf_len(paths); ++i) {
		draw_path(paths[i], DARK_BLUE);
	}
}

Vec2 *path_copy(BUF(Vec2 *path)) {
	BUF(Vec2 *result) = 0;
	for (int i=0; i<buf_len(path); ++i) {
		buf_push(result, path[i]);
	}
	return result;
}

void hike_vis(Trailhead *trailhead, BUF(int **grid), 
		      int x, int y, BUF(Vec2 *path))//, BUF(Vec2 **paths))
{
	bool unique = true;
	for (int i=0; i<buf_len(path); ++i) {
		if (path[i].x == x && path[i].y == y) {
			unique = false;
			break;
		}
	}
	if (unique) {
		buf_push(path, (Vec2){x,y});
		draw_grid(grid);
		draw_paths(paths);
		draw_path(path, BLUE);
		if (frames++ % SKIP_FRAMES == 0)
			nextframe();
	}

	if (grid[y][x] == 9) {
		buf_push(paths, path_copy(path));
		++trailhead->trails;
		for (int i=0; i<buf_len(trailhead->peaks); ++i) {
			if (trailhead->peaks[i].x == x && trailhead->peaks[i].y == y) {
				return;
			}
		}
		buf_push(trailhead->peaks, (Vec2){x,y});
		return;
	}

	int path_len = buf_len(path);
	if (is_on_grid(grid, x+1, y) && grid[y][x+1] == grid[y][x] + 1) {
		hike_vis(trailhead, grid, x+1, y, path);
		buf_set_len(path, path_len);
	}
	if (is_on_grid(grid, x-1, y) && grid[y][x-1] == grid[y][x] + 1) {
		hike_vis(trailhead, grid, x-1, y, path);
		buf_set_len(path, path_len);
	}
	if (is_on_grid(grid, x, y+1) && grid[y+1][x] == grid[y][x] + 1) {
		hike_vis(trailhead, grid, x, y+1, path);
		buf_set_len(path, path_len);
	}
	if (is_on_grid(grid, x, y-1) && grid[y-1][x] == grid[y][x] + 1) {
		hike_vis(trailhead, grid, x, y-1, path);
		buf_set_len(path, path_len);
	}
}



void visualize(Topography topo) {
	setupgif(0, 2, "hoof.gif");

	cell_size = SCREEN_SIZE / buf_len(topo.grid);
	pad = (SCREEN_SIZE % buf_len(topo.grid)) / 2;

	BUF(Vec2 *path) = NULL;
	for (int i=0; i<buf_len(topo.trailheads); ++i) {
		buf_set_len(path, 0);
		Trailhead *trailhead = &topo.trailheads[i];
		hike_vis(trailhead, topo.grid, trailhead->x, trailhead->y, path);
	}

	for (int i=0; i<25; ++i) nextframe();

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

	Topography topo = {0};

	int x = 0, y = 0;
	for (char *c = file_data; *c != 0; ++c) {
		BUF(int *row) = NULL;
		while (*c != '\n') {
			int height = *c - '0';
			buf_push(row, height);
			if (height == 0) {
				buf_push(topo.trailheads, (Trailhead){.x=x,.y=y});
			} 
			++c;
			++x;
		}
		buf_push(topo.grid, row);
		++y;
		x = 0;
	}

	int scores = 0;
	int ratings = 0;
	for (int i=0; i<buf_len(topo.trailheads); ++i) {
		Trailhead *trailhead = &topo.trailheads[i];
		hike(trailhead, topo.grid, trailhead->x, trailhead->y);
		scores += buf_len(trailhead->peaks);
		ratings += trailhead->trails;
	}
	printf("part one: %d\n", scores);
	printf("part two: %d\n", ratings);

	visualize(topo);

	return 0;
}

