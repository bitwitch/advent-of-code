#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE 1024
#define WHITE       0xffffff
#define GREY        0xa2a2a2
#define GREEN       0x05fc11 

typedef struct {
	int x, y;
} Vec2;

typedef struct TimelinePos TimelinePos; 
struct TimelinePos {
	Vec2 pos;
	TimelinePos *next;
}; 

typedef struct {
	Vec2 pos;
	int len;
	int delay;
	bool done;
} Beam;

typedef struct {
	BUF(Vec2 *splitters);
	BUF(Beam *beams);
	int rows, cols;
	char *grid;
} Tachyon;

bool beam_at(Tachyon t, int beam_index, int x, int y) {
	for (int i=0; i<buf_len(t.beams); ++i) {
		if (i == beam_index) continue;
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
	int grid_width  = cell_size * t.cols;
	int grid_height = cell_size * t.rows;
	int off_x = (SCREEN_SIZE - grid_width) / 2;
	int off_y = (SCREEN_SIZE - grid_height) / 2;
	for (int i=0; i<buf_len(t.splitters); ++i) {
		int x = off_x + t.splitters[i].x * cell_size;
		int y = off_y + t.splitters[i].y * cell_size + cell_size;
		drawtri(x, y, x+cell_size, y, x+(cell_size/2), y-cell_size, GREY);
	}

	int beam_width = 1;
	for (int i=0; i<buf_len(t.beams); ++i) {
		Beam beam = t.beams[i];
		if (beam.len > 0) {
			int x = off_x + beam.pos.x * cell_size + (cell_size/2) - (beam_width/2);
			int y0 = off_y + beam.pos.y * cell_size;
			int y1 = off_y + (beam.pos.y + beam.len) * cell_size;
			drawline(x, y0, x, y1, beam_width, GREEN);
		}
	}
}

void draw_quantum_beam(Tachyon t, BUF(Vec2 *path)) {
	if (buf_len(path) == 0) return;
	int cell_size = MIN(SCREEN_SIZE/t.rows, SCREEN_SIZE/t.cols);
	int grid_width  = cell_size * t.cols;
	int grid_height = cell_size * t.rows;
	int off_x = (SCREEN_SIZE - grid_width) / 2;
	int off_y = (SCREEN_SIZE - grid_height) / 2;
	int beam_width = 1;
	for (int i=0; i<buf_len(path)-1; ++i) {
		Vec2 prev_pos = path[i];
		Vec2      pos = path[i+1];
		int x0 = off_x + prev_pos.x * cell_size + (cell_size/2) - (beam_width/2);
		int x1 = off_x +      pos.x * cell_size + (cell_size/2) - (beam_width/2);
		int y0 = off_y + prev_pos.y * cell_size;
		int y1 = off_y +      pos.y * cell_size;
		drawline(x0, y0, x1, y1, beam_width, GREEN);
	}
}

void draw_count(int count) {
	int x = 900;
	int y = 25;
	drawbox(x, y, 100, 50, 0x000000);
	drawstringf(x, y, 3, GREY, "%4d", count);
}

void part_one(Tachyon t) {
#define PART_ONE_GIF
#ifdef PART_ONE_GIF
	setupgif(1, 1, "labs.gif");

	clear();
	draw_tachyon(t);
	nextframe();
#endif

	int result = 0;
	bool all_beams_done = false;
	while (!all_beams_done) {
		all_beams_done = true;
		for (int i=0; i<buf_len(t.beams); ++i) {
			Beam *b = &t.beams[i];
			if (!b->done) {
				if (b->delay > 0) {
					b->delay -= 1;
				} else {
					b->len += 1;
					Vec2 p = {b->pos.x, b->pos.y + b->len};
					if (p.y >= t.rows || beam_at(t, i, p.x, p.y)) {
						b->done = true;
					} else if (t.grid[p.y * t.cols + p.x] == '^') {
						// split beam
						result += 1;
						b->done = true;
						if (p.x - 1 >= 0 && !beam_at(t, i, p.x-1, p.y)) {
							buf_push(t.beams, (Beam){.pos = {p.x-1, p.y}, .delay=3});
						}
						if (p.x + 1 < t.cols && !beam_at(t, i, p.x+1, p.y)) {
							buf_push(t.beams, (Beam){.pos = {p.x+1, p.y}, .delay=3});
						}
					}
				}
				if (!b->done) all_beams_done = false;
			}
		}
#ifdef PART_ONE_GIF
		draw_tachyon(t);
		draw_count(result);
		nextframe();
#endif
	}

	for (int i=0; i<8; ++i) nextframe();
#ifdef PART_ONE_GIF
	endgif();
#endif
	printf("part one: %d\n", result);
}

TimelinePos *timeline_create(Arena *arena, Vec2 pos) {
	TimelinePos *tp = arena_alloc_zeroed(arena, sizeof(TimelinePos));
	tp->pos = pos;
	return tp;
}

void timeline_push(TimelinePos **head, TimelinePos *p) {
	if (*head == NULL) {
		*head = p;
		return;
	}
	TimelinePos *node;
	for (node = *head; node->next; node = node->next);
	node->next = p;
}

int traverse(Tachyon t, Vec2 pos) {
	for (; pos.y < t.rows; pos.y += 1) {
		char c = t.grid[pos.y * t.cols + pos.x];
		if (c == '^') {
			int left_count = traverse(t, (Vec2){pos.x-1, pos.y});
			int right_count = traverse(t, (Vec2){pos.x+1, pos.y});
			return left_count + right_count;
		} 
	}
	return 1;
}

void part_two_2(Tachyon t) {
	int result = traverse(t, t.beams[0].pos);
	printf("part two: %d\n", result);
}

void part_two(Tachyon t) {
	// clear beams for drawing
	buf_set_len(t.beams, 0); 

	// setupgif(1, 1, "labs.gif");

	// clear();
	// draw_tachyon(t);
	// nextframe();

	U64 result = 0;

	BUF(Vec2 *timelines) = NULL;
	buf_set_cap(timelines, 100);
	buf_push(timelines, t.beams[0].pos);

	// BUF(Vec2 *path) = NULL;

	while (buf_lenu(timelines) > 0) {
		if (result % 10000000 == 0) {
			printf("                                    \r");
			printf("timelines=%llu queue=%llu", result, buf_lenu(timelines));
		}
		U64 last = buf_lenu(timelines)-1;
		Vec2 pos = timelines[last];
		buf_set_len(timelines, last);

		// result += 1;

		// int reset_path_len = buf_len(path);

		// clear();
		// draw_tachyon(t);
		// draw_quantum_beam(t, path);
		// nextframe();

		for (; pos.y < t.rows; pos.y += 1) {
			// buf_push(path, pos);
			// draw_quantum_beam(t, path);
			// nextframe();

			char c = t.grid[pos.y * t.cols + pos.x];
			if (c == '^') {
				Vec2 left = (Vec2){pos.x-1, pos.y};
				Vec2 right = (Vec2){pos.x+1, pos.y};
				buf_push(timelines, right);
				buf_push(timelines, left);
				break;
			} 
		}
		if (pos.y >= t.rows) {
			result += 1;
		}
	}

	printf("part two: %llu\n", result);

	// endgif();
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
	// part_two(t);

	return 0;
}
