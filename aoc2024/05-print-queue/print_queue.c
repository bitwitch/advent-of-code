#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

#define ROW_HEIGHT 112
#define ROW_WIDTH  100
#define UNIT 2
#define PAD 2
#define NUM_COLS  11
#define RULE_X 500
#define SCANLINE_SPEED 10

#define WHITE      0xFFFFFFFF
#define RED        0xFF1904D1
#define RULE_COLOR 0xFF00FF00

typedef struct {
	int x, y, w, h;
	int target_x, target_y;
	bool moving;
	U32 color;
} Bar;

typedef struct {
	BUF(Bar *bars);
	bool valid;
	bool tint_enabled;
	U32 tint_color;
} GUpdate;

typedef struct {
	U64 x, y;
} Rule;

bool index_violates_rules(BUF(Rule *rules), BUF(U64 *update), int check_index, int *index_of_violation) {
	U64 val = update[check_index];
	for (int j=0; j<buf_len(rules); ++j) {
		if (val == rules[j].x) {
			for (int i=check_index-1; i>=0; --i) {
				if (update[i] == rules[j].y) {
					if (index_of_violation) *index_of_violation = i;
					return true;
				}
			}
		}
	}
	return false;
}

void part_one(BUF(Rule *rules), BUF(U64 **updates)) {
	BUF(int *correct_ids) = NULL;
	for (int j=0; j<buf_len(updates); ++j) {
		bool valid = true;
		for (int i=0; i<buf_len(updates[j]); ++i) {
			if (index_violates_rules(rules, updates[j], i, NULL)) {
				valid = false;
				break;
			}	
		}
		if (valid) {
			buf_push(correct_ids, j);
		}
	}

	U64 sum = 0;
	for (int i=0; i<buf_len(correct_ids); ++i) {
		BUF(U64 *update) = updates[correct_ids[i]];
		int mid = (int)buf_len(update) / (int)2;
		sum += update[mid];
	}
	printf("part one: %llu\n", sum);
}

U64 *buf_copy(BUF(U64 *source)) {
	U64 *result = NULL;
	for (int i=0; i<buf_len(source); ++i) {
		buf_push(result, source[i]);
	}
	return result;
}

void sort_by_rules(BUF(U64 *update), BUF(Rule *rules)) {
	while (true) {
		bool sorted = true;
		for (int i=0; i<buf_len(update); ++i) {
			int other;
			if (index_violates_rules(rules, update, i, &other)) {
				U64 tmp = update[i];
				update[i] = update[other];
				update[other] = tmp;
				sorted = false;
			}	
		}

		if (sorted) break;
	}
}

void part_two(BUF(Rule *rules), BUF(U64 **updates)) {
	BUF(int *incorrect_ids) = NULL;
	for (int j=0; j<buf_len(updates); ++j) {
		bool valid = true;
		for (int i=0; i<buf_len(updates[j]); ++i) {
			if (index_violates_rules(rules, updates[j], i, NULL)) {
				valid = false;
				break;
			}	
		}
		if (!valid) {
			buf_push(incorrect_ids, j);
		}
	}

	U64 sum = 0;
	for (int i=0; i<buf_len(incorrect_ids); ++i) {
		BUF(U64 *update) = buf_copy(updates[incorrect_ids[i]]);
		sort_by_rules(update, rules);
		int mid = (int)buf_len(update) / (int)2;
		sum += update[mid];
	}

	printf("part two: %llu\n", sum);
}

U32 color_from_height(int h) {
	U8 hue = 0xFF - (U8)(h*2);
	U32 color = 0xFFFF00FF | (hue << 8);
	return color;
}

U32 tint_color(U32 color, U32 tint) {
	// U32 result = color & tint;
	U32 result = color * tint;
	return result;
}


U32 blend_colors(U32 a, U32 b) {
	// U8 r = 
	return a + b;
}

void draw_updates(BUF(GUpdate *updates)) {
	for (int j=0; j<buf_len(updates); ++j) {
		GUpdate update = updates[j];
		for (int i=0; i<buf_len(update.bars); ++i) {
			Bar bar = update.bars[i];
			U32 color = bar.color;
			if (update.tint_enabled) {
				color = tint_color(color, update.tint_color);
			}
			drawbox(bar.x, bar.y, UNIT, bar.h, color);
		}
	}
}

GUpdate *make_g_updates(BUF(U64 **updates), BUF(Rule *rules)) {
	BUF(GUpdate *g_updates) = NULL;
	for (int j=0; j<buf_len(updates); ++j) {
		GUpdate gu = {0};
		gu.valid = true;
		int off_x = (j % NUM_COLS) * ROW_WIDTH;
		int off_y = (j / NUM_COLS) * ROW_HEIGHT;
		for (int i=0; i<buf_len(updates[j]); ++i) {
			if (index_violates_rules(rules, updates[j], i, NULL)) {
				gu.valid = false;
			}
			int h = (int)updates[j][i];
			U8 hue = 0xFF - (U8)(h*2);
			U32 color = 0xFFFF00FF | (hue << 8);
			Bar bar = {
				.x = i * (UNIT + PAD) + off_x,
				.y = off_y,
				.w = UNIT,
				.h = h,
				.color = color,
			};
			buf_push(gu.bars, bar);
		}
		buf_push(g_updates, gu);
	}
	return g_updates;
}

void move_updates(BUF(GUpdate *updates)) {
	for (int j=0; j<buf_len(updates); ++j) {
	}
}

void highlight_valid_updates(BUF(GUpdate *updates), int scanline) {
	for (int j=0; j<buf_len(updates); ++j) {
		GUpdate *update = &updates[j];
		int update_center_y = (j / NUM_COLS) * ROW_HEIGHT + (ROW_HEIGHT/2);
		if (scanline >= update_center_y) {
			update->tint_enabled = true;
			update->tint_color = update->valid ? 0xFF00FF00 : 0xFF0000FF;
		}
	}
}

void visualize(BUF(Rule *rules), BUF(U64 **updates)) {
	setupgif(0, 1, "print_queue.gif");
	
	BUF(GUpdate *g_updates) = make_g_updates(updates, rules);

	for (int scanline=0; scanline<SCREEN_HEIGHT; scanline+=SCANLINE_SPEED) {

		clear();
		draw_updates(g_updates);
		drawbox(0, scanline, SCREEN_WIDTH, 2, RED);
		nextframe();

		highlight_valid_updates(g_updates, scanline);

		move_updates(g_updates);
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

	init_lexer(file_name, file_data);

	BUF(Rule *rules) = NULL;
	while (!is_token(TOKEN_EOF) && !match_token('\n')) {
		Rule rule = {0};
		rule.x = parse_int();
		expect_token('|');
		rule.y = parse_int();
		expect_token('\n');
		buf_push(rules, rule);
	} 

	while (match_token('\n')) {}

	BUF(U64 **updates) = NULL;
	while (!is_token(TOKEN_EOF)) {
		if (is_token(TOKEN_INT)) {
			BUF(U64 *update) = NULL;
			do {
				buf_push(update, parse_int());
			} while (match_token(','));
			buf_push(updates, update);
		} else {
			next_token();
		}
	}

	part_one(rules, updates);
	part_two(rules, updates);

	visualize(rules, updates);

	return 0;
}

