#include "../common.c"

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	BUF(Vec2 *tiles);
	int rows, cols;
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

void part_two(Grid grid) {
	int result = 0;
	// printf("part two: %d\n", result);
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
		if (*line == '\r' || *line == 0) break;
		Vec2 t = {0};
		sscanf(line, "%d,%d", &t.x, &t.y);
		buf_push(grid.tiles, t);
		if (t.x > grid.cols) grid.cols = t.x;
		if (t.y > grid.rows) grid.rows = t.y;
	}

	part_one(grid);
	part_two(grid);

	return 0;
}
