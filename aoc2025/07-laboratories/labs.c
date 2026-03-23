#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE 1024
#define GREY        0x9A9A9A
#define GREEN       0x02e20d

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	Vec2 pos;
	int len;
	bool done;
} Beam;

typedef struct {
	BUF(Vec2 *splitters);
	BUF(Beam *beams);
	int rows, cols;
	char *grid;
} Tachyon;

bool beam_at(Tachyon t, int x, int y) {
	for (int i=0; i<buf_len(t.beams); ++i) {
		Beam b = t.beams[i];
		if (b.pos.x == x) {
			for (int by = b.pos.y; by <= b.pos.y + b.len; ++by) {
				if (by == y) {
					return true;
				}
			}
		}
	}
	return false;
}

void draw_tachyon(Tachyon t) {
	int cell_size = MIN(SCREEN_SIZE/t.rows, SCREEN_SIZE/t.cols);
	for (int i=0; i<buf_len(t.splitters); ++i) {
		int x = t.splitters[i].x * cell_size;
		int y = t.splitters[i].y * cell_size + cell_size;
		drawtri(x, y, x+cell_size, y, x+(cell_size/2), y-cell_size, GREY);
	}

	int beam_width = 1;
	for (int i=0; i<buf_len(t.beams); ++i) {
		Beam beam = t.beams[i];
		int x = beam.pos.x * cell_size + (cell_size/2) - (beam_width/2);
		int y0 = beam.pos.y * cell_size;
		int y1 = (beam.pos.y + beam.len) * cell_size;
		drawline(x, y0, x, y1, beam_width, GREEN);
	}
}

void part_one(Tachyon t) {
	setupgif(1, 1, "labs.gif");

	clear();
	draw_tachyon(t);
	nextframe();

	int result = 0;
	int row = 1;
	bool all_beams_done = false;
	while (!all_beams_done) {
		all_beams_done = true;
		for (int i=0; i<buf_len(t.beams); ++i) {
			Beam *b = &t.beams[i];
			if (!b->done) {
				while (b->pos.y + b->len < row) {
					Vec2 p = {b->pos.x, b->pos.y + b->len + 1};
					if (p.y >= t.rows || beam_at(t, p.x, p.y)) {
						b->done = true;
					} else if (t.grid[p.y * t.cols + p.x] == '^') {
						// split beam
						result += 1;
						b->done = true;
						if (p.x - 1 >= 0 && !beam_at(t, p.x-1, p.y)) {
							buf_push(t.beams, (Beam){.pos = {p.x-1, p.y}});
						}
						if (p.x + 1 < t.cols && !beam_at(t, p.x+1, p.y)) {
							buf_push(t.beams, (Beam){.pos = {p.x+1, p.y}});
						}
					}
					b->len += 1;
					if (b->done) break;
				}
				if (!b->done) all_beams_done = false;
			}
		}
		draw_tachyon(t);
		nextframe();
		row += 1;
	}

	endgif();
	printf("part one: %d\n", result);
}

void part_two(Tachyon t) {
	int result = 0;
	// printf("part two: %d\n", result);
}

Tachyon parse_tachyon(StringView file_data) {
	Tachyon t = {0};
	int y = 0;
	int max_x = 0;
	for (;;) {
		StringView line = sv_chop_line(&file_data);
		if (line.length == 0) break;
		if (line.length > max_x) max_x = line.length;
		for (int x = 0; x < line.length; ++x) {
			if (line.data[x] == 'S') {
				buf_push(t.beams, (Beam){.pos = {x, y}});
			} else if (line.data[x] == '^') {
				buf_push(t.splitters, (Vec2){x, y});
			}
		}
		y += 1;
	}
	t.rows = y - 1;
	t.cols = max_x;
	t.grid = xcalloc(t.rows * t.cols, 1);
	for (int i=0; i<buf_len(t.beams); ++i) {
		Beam b = t.beams[i];
		t.grid[b.pos.y * t.cols + b.pos.x] = 'S';
	}
	for (int i=0; i<buf_len(t.splitters); ++i) {
		Vec2 s = t.splitters[i];
		t.grid[s.y * t.cols + s.x] = '^';
	}
	return t;
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

	StringView sv_file_data = sv_from_cstr(file_data);
	Tachyon t = parse_tachyon(sv_file_data);

	part_one(t);
	part_two(t);

	return 0;
}
