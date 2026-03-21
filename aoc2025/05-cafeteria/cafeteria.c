#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE 1024
#define CELL_PAD    1
#define CELL_SIZE   5
#define SCREEN_COLS (SCREEN_SIZE / (CELL_SIZE + CELL_PAD))
#define GREEN       0x11af11
#define RED         0x1212cc
#define GOLD        0x17e9ed
#define GREY        0x393939

typedef struct {
	U64 min, max;
	bool removed;
} Range;

typedef struct {
	BUF(Range *fresh);
	BUF(U64 *ingredients);
} Database;

void part_one(Database db) {
	int result = 0;
	for (int i=0; i<buf_len(db.ingredients); ++i) {
		U64 id = db.ingredients[i];
		for (int j=0; j<buf_len(db.fresh); ++j) {
			Range range = db.fresh[j];
			if (id >= range.min && id <= range.max) {
				result += 1;
				break;
			}
		}
	}
	printf("part one: %d\n", result);
}

int range_cmp(const void *_a, const void *_b) {
	Range *a = (Range *)_a;
	Range *b = (Range *)_b;
	if (a->min < b->min) {
		return -1;
	} else if (a->min > b->min) {
		return 1;
	} else {
		if (a->max < b->max) {
			return -1;
		} else if (a->max > b->max) {
			return 1;
		} else {
			return 0;
		}
	}
}

void part_two(Database db) {
	U64 result = 0;

	qsort(db.fresh, buf_len(db.fresh), sizeof(db.fresh[0]), range_cmp);

	for (int i=0; i<buf_len(db.fresh); ++i) {
		for (int j=i+1; j<buf_len(db.fresh); ++j) {
			if (i == j) continue;
			Range *a = &db.fresh[i];
			Range *b = &db.fresh[j];
			if (a->removed || b->removed) continue;
			if (b->min >= a->min && b->max <= a->max) {
				b->removed = true;
			} else if (b->min >= a->min && b->min <= a->max) {
				b->min = a->max + 1;
			}
		}
	}


	for (int i=0; i<buf_len(db.fresh); ++i) {
		if (!db.fresh[i].removed) {
			result += db.fresh[i].max - db.fresh[i].min + 1;
		}
	}

	printf("part two: %llu\n", result);
}

int map_to_screen(U64 val, U64 min, U64 max) {
	F64 t = (F64)(val - min) / (F64)(max - min);
	return (int)(t * SCREEN_COLS * SCREEN_COLS);
}

U64 map_to_range(int val, U64 min, U64 max) {
	F64 t = (F64)(val) / (F64)(SCREEN_COLS * SCREEN_COLS);
	return (U64)(t * (max - min) + min);
}

bool is_fresh(Database db, U64 id) {
	for (int i=0; i<buf_len(db.fresh); ++i) {
		if (id >= db.fresh[i].min && id <= db.fresh[i].max) 
			return true;
	}
	return false;
}

void visualize(Database db) {
	setupgif(1, 1, "cafeteria.gif");

	U64 min = UINT64_MAX;
	U64 max = 0;
	for (int i=0; i<buf_len(db.fresh); ++i) {
		if (db.fresh[i].min < min) min = db.fresh[i].min;
		if (db.fresh[i].max > max) max = db.fresh[i].max;
	}

	int num_pixels = SCREEN_COLS * SCREEN_COLS;
	for (int i=0; i<num_pixels; ++i) {
		U64 val = map_to_range(i, min, max);
		U32 color = is_fresh(db, val) ? GREEN : GREY;
		int x = 3 + (i % SCREEN_COLS) * (CELL_SIZE + CELL_PAD);
		int y = 3 + (i / SCREEN_COLS) * (CELL_SIZE + CELL_PAD);
		drawbox(x, y, CELL_SIZE, CELL_SIZE, color);
	}

	 nextframe();
	 nextframe();

	for (int i=0; i<buf_len(db.ingredients); ++i) {
		U64 id = db.ingredients[i];
		int pixel_index = map_to_screen(id, min, max);
		U32 color = RED;
		for (int j=0; j<buf_len(db.fresh); ++j) {
			Range range = db.fresh[j];
			if (id >= range.min && id <= range.max) {
				color = GOLD;
				break;
			}
		}

		int x = 3 + (pixel_index % SCREEN_COLS) * (CELL_SIZE + CELL_PAD);
		int y = 3 + (pixel_index / SCREEN_COLS) * (CELL_SIZE + CELL_PAD);
		drawbox(x, y, CELL_SIZE, CELL_SIZE, color);
		nextframe();
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

	Database db = {0};

	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == 0 || *line == '\r') break;
		Range range = {0};
		if (sscanf(line, "%llu-%llu", &range.min, &range.max) != 2) {
			fatal("failed to parse range: %s", line);
		}
		buf_push(db.fresh, range);
	}

	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == 0 || *line == '\r') break;
		U64 id = 0;
		if (sscanf(line, "%llu", &id) != 1) {
			fatal("failed to parse ingredient id: %s", line);
		}
		buf_push(db.ingredients, id);
	}

	Database db_copy = {0};
	for (int i=0; i<buf_len(db.fresh); ++i)       buf_push(db_copy.fresh, db.fresh[i]);
	for (int i=0; i<buf_len(db.ingredients); ++i) buf_push(db_copy.ingredients, db.ingredients[i]);

	part_one(db);
	part_two(db);
	visualize(db_copy);

	return 0;
}
