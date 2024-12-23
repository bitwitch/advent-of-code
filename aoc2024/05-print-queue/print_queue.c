#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_WIDTH   1024
#define SCREEN_HEIGHT  1024

#define ROW_HEIGHT     112
#define ROW_WIDTH      100
#define BAR_W          2
#define BAR_SPACING    2
#define NUM_COLS       11
#define NUM_ROWS       9
#define RULE_X         500
#define SCANLINE_SPEED 10
#define OFF_Y          10

#define WHITE 0xFFDEDEDE
#define RED   0xFF1904D1
#define GOLD  0xFF01CAF8

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
	int hue_rotate;
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
	U32 color = 0xFF130013 | (hue << 8);
	return color;
}

U32 hue_rotate_right(U32 color, int num_bytes) {
	U8 r = color & 0xFF;
	U8 g = (color >> 8)  & 0xFF;
	U8 b = (color >> 16) & 0xFF;
	if ((num_bytes % 3) == 1) {
		color = 0xFF000000 | (r << 16) | (b << 8) | g;
	} else if ((num_bytes % 3) == 2) {
		color = 0xFF000000 | (g << 16) | (r << 8) | b;
	}
	return color;
}

U32 tint_color(U32 color, U32 tint) {
	// U32 result = color & tint;
	// U32 result = color * tint;

	F32 tint_r = (tint & 0xFF) / 0x90;
	F32 tint_g = ((tint >> 8)  & 0xFF) / 0x90;
	F32 tint_b = ((tint >> 16) & 0xFF) / 0x90;

	U32 color_r = MIN(0xFF, (color & 0xFF) * tint_r);
	U32 color_g = MIN(0xFF, ((color >> 8)  & 0xFF) * tint_g);
	U32 color_b = MIN(0xFF, ((color >> 16) & 0xFF) * tint_b);

	U32 result = 0xFF000000 | ((U8)color_b << 16) | ((U8)color_g << 8) | (U8)color_r;
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
				color = hue_rotate_right(color, update.hue_rotate);
			}
			drawbox(bar.x, bar.y, BAR_W, bar.h, color);
		}
	}
}

GUpdate *make_g_updates(BUF(U64 **updates), BUF(Rule *rules)) {
	BUF(GUpdate *g_updates) = NULL;
	for (int j=0; j<buf_len(updates); ++j) {
		int row = (j / NUM_COLS);
		if (row >= NUM_ROWS) break;

		GUpdate gu = {0};
		gu.valid = true;
		int bar_y = row * ROW_HEIGHT + OFF_Y;
		int off_x = (j % NUM_COLS) * ROW_WIDTH;
		for (int i=0; i<buf_len(updates[j]); ++i) {
			if (index_violates_rules(rules, updates[j], i, NULL)) {
				gu.valid = false;
			}
			int h = (int)updates[j][i];
			U32 color = color_from_height(h);
			Bar bar = {
				.x = i * (BAR_W + BAR_SPACING) + off_x,
				.y = bar_y,
				.w = BAR_W,
				.h = h,
				.color = color,
			};
			buf_push(gu.bars, bar);
		}
		buf_push(g_updates, gu);
	}
	return g_updates;
}

void highlight_valid_updates(BUF(GUpdate *updates), int scanline) {
	for (int j=0; j<buf_len(updates); ++j) {
		GUpdate *update = &updates[j];
		int update_center_y = (j / NUM_COLS) * ROW_HEIGHT + (ROW_HEIGHT/2);
		if (scanline >= update_center_y) {
			update->tint_enabled = true;
			update->hue_rotate = update->valid ? 0 : 1;
		}
	}
}

void highlight_middle_values(BUF(GUpdate *updates), int scanline) {
	for (int j=0; j<buf_len(updates); ++j) {
		GUpdate *update = &updates[j];
		int update_center_y = (j / NUM_COLS) * ROW_HEIGHT + (ROW_HEIGHT/2);
		if (update->valid && scanline >= update_center_y) {
			int mid = buf_len(update->bars) / 2;
			update->bars[mid].color = GOLD;
		}
	}
}

void visualize(BUF(Rule *rules), BUF(U64 **updates)) {
	setupgif(0, 1, "print_queue.gif");
	
	BUF(GUpdate *g_updates) = make_g_updates(updates, rules);

	for (int scanline=0; scanline<SCREEN_HEIGHT; scanline+=SCANLINE_SPEED) {
		clear();
		draw_updates(g_updates);
		drawbox(0, scanline, SCREEN_WIDTH, 2, WHITE);
		nextframe();

		highlight_valid_updates(g_updates, scanline);
	}

	for (int scanline=0; scanline<SCREEN_HEIGHT; scanline+=SCANLINE_SPEED) {
		clear();
		draw_updates(g_updates);
		drawbox(0, scanline, SCREEN_WIDTH, 2, WHITE);
		nextframe();

		highlight_middle_values(g_updates, scanline);
	}

	// for (int i=0; i<25; ++i) nextframe();

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

