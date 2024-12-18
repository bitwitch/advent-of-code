#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

#define WHITE 0xFFFFFFFF
#define ROW_HEIGHT 112
#define ROW_WIDTH  100
#define UNIT 2
#define PAD 2

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

void draw_updates(BUF(U64 **updates)) {
	int num_rows = (int)(SCREEN_HEIGHT / ROW_HEIGHT);
	for (int j=0; j<buf_len(updates); ++j) {
		int y = (j % num_rows) * ROW_HEIGHT;
		int off_x = (j / num_rows) * (int)ROW_WIDTH;
		for (int i=0; i<buf_len(updates[j]); ++i) {
			int x = i * (UNIT + PAD) + off_x;
			int h = (int)updates[j][i];
			U8 hue = 0xFF - (U8)(h*2);
			U32 color = 0xFFFF00FF | (hue << 8);
			drawbox(x, y, UNIT, h, color);
		}
	}
}

void visualize(BUF(Rule *rules), BUF(U64 **updates)) {
	setupgif(0, 1, "print_queue.gif");

	clear();
	draw_updates(updates);
	nextframe();

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

