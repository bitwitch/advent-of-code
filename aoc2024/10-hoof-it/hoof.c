#include "../common.c"

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

	return 0;
}

