#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#if _WIN32
#define LINE_END "\r\n"
#else
#define LINE_END "\n"
#endif

#define SCREEN_SIZE  1024
#define FONT_SIZE    3
#define TEXT_OFF     7
#define CELL_HEIGHT  50
#define CELL_WIDTH   (FONT_SIZE*8)
#define PAUSE_FRAMES 3
#define WHITE        0xeff2f2
#define BLACK        0x000000
#define GREEN        0x008a00

#define BORDER          50
#define DIVIDER_MARGIN  120
#define LAST_COL_END    420

typedef enum {
	OP_NONE,
	OP_ADD,
	OP_MUL,
} Op;

typedef struct {
	Op op;
	BUF(S64 *nums); 
	S64 solution;
} Equation;

typedef struct {
	int x, y;
	U32 color;
	char *text;
} Marking;

Op op_from_char(char c) {
	switch (c) {
		case '+': return OP_ADD;
		case '*': return OP_MUL;
		default:  return OP_NONE;
	}
}

char *str_from_op(Op op) {
	switch (op) {
		case OP_ADD: return "+";
		case OP_MUL: return "*";
		default:  return "";
	}
}

U64 sum(BUF(Equation *equations)) {
	U64 result = 0;

	for (int i=0; i<buf_len(equations); ++i) {
		Op op = equations[i].op;
		U64 eq_result;
		switch (op) {
			case OP_ADD: eq_result = 0;     break;
			case OP_MUL: eq_result = 1;     break;
			default: assert("unreachable"); break;
		}

		for (int j=0; j<buf_len(equations[i].nums); ++j) {
			U64 num = equations[i].nums[j];
			switch (op) {
				case OP_ADD: eq_result += num;  break;
				case OP_MUL: eq_result *= num;  break;
				default: assert("unreachable"); break;
			}
		}

		equations[i].solution = eq_result;
		result += eq_result;
	}

	return result;
}


void part_one(BUF(Equation *equations)) {
	U64 result = sum(equations);
	printf("part one: %llu\n", result);
}

void part_two(BUF(Equation *equations)) {
	U64 result = sum(equations);
	printf("part two: %llu\n", result);
}

BUF(Equation *) parse_equations(char *file_data) {
	BUF(Equation *equations) = NULL;
	// get number of equations from first line
	char *line = chop_by_delimiter(&file_data, "\n");
	for (;;) {
		char *num_str = chop_by_delimiter(&line, " ");
		if (*num_str == '\r' || *num_str == 0) break;
		Equation e = {0};
		S64 num = 0;
		if (sscanf(num_str, "%lld", &num) != 1) {
			fatal("failed to parse num: %s\n", num_str);
		}
		buf_push(e.nums, num);
		buf_push(equations, e);
		eat_spaces(&line);
	}

	// parse remaining values in equations
	for (;;) {
		line = chop_by_delimiter(&file_data, "\n");
		eat_spaces(&line);
		if (*line == '\r' || *line == 0) break;
		if (*line >= '0' && *line <= '9') { 
			for(int i=0; ; ++i) {
				char *num_str = chop_by_delimiter(&line, " ");
				if (*num_str == '\r' || *num_str == 0) break;
				S64 num = 0;
				if (sscanf(num_str, "%lld", &num) != 1) {
					fatal("failed to parse num: %s\n", num_str);
				}
				buf_push(equations[i].nums, num);
				eat_spaces(&line);
			}
		} else {
			for(int i=0; ; ++i) {
				char *op_str = chop_by_delimiter(&line, " ");
				if (*op_str == '\r' || *op_str == 0) break;
				Op op = op_from_char(*op_str);
				assert(op != OP_NONE);
				equations[i].op = op;
				eat_spaces(&line);
			}
		}
	}
	return equations;
}

U64 pow_ten(int exp) {
	U64 result = 1;
	for (int i=0; i<exp; ++i) result *= 10;
	return result;
}

BUF(Equation *) parse_ceph_equations(char *file_data) {
	BUF(Equation *equations) = NULL;
	
	// count num equations and num cols
	int num_cols = 0;
	bool was_digit = false;
	for (char *line = file_data; *line != '\r' && *line != '\n'; ++line) {
		bool is_digit = isdigit(*line);
		if (is_digit && !was_digit) buf_push(equations, (Equation){0});
		num_cols += 1;
		was_digit = is_digit;
	}

	// count num digits
	int max_digits = 0;
	for (char *c = file_data; *c != 0; ++c) {
		if (op_from_char(*c) != OP_NONE) break;
		max_digits += 1;
		while (*c != '\n') c += 1;
	}

	// count num operands
	int stride = num_cols + (int)strlen(LINE_END);
	int eq_i = buf_len(equations) - 1;
	Equation *e = &equations[eq_i];
	for (int col = num_cols-1; col >= 0; --col) {
		bool next_eq = true;
		for (int k=0; k<max_digits; ++k) {
			int row = k;
			char c = file_data[row * stride + col];
			if (isdigit(c)) {
				buf_push(e->nums, 0);
				next_eq = false;
				break;
			}
		}
		if (next_eq) {
			e = &equations[--eq_i];
		}
	}

	// parse values
	int col = num_cols - 1;
	for (int i=buf_len(equations); i>0; --i) {
		Equation *e = &equations[i-1];
		for (int j=0; j<buf_len(e->nums); ++j) {
			BUF(U64 *digits) = NULL;
			for (int k=0; k<max_digits; ++k) {
				int row = k;
				char c = file_data[row * stride + col];
				if (isdigit(c)) {
					buf_push(digits, c - '0');
				}
			}

			int exp = buf_len(digits) - 1;
			for (int k=0; k<buf_len(digits); ++k) {
				e->nums[j] += pow_ten(exp) * digits[k];
				exp -= 1;
			}
			col -= 1;
		}
		e->op = op_from_char(file_data[max_digits * stride + col + 1]);
		col -= 1;
	}

	return equations;
}

void draw_bg(BUF(char **lines)) {
	drawrect(BORDER, 0, SCREEN_SIZE - BORDER, SCREEN_SIZE, WHITE);
	// vertical divider
	drawline(SCREEN_SIZE-BORDER-DIVIDER_MARGIN, 0, SCREEN_SIZE-BORDER-DIVIDER_MARGIN, SCREEN_SIZE, 1, 0x8d72aa);
	// horizontal dividers
	for (int y=DIVIDER_MARGIN; y<SCREEN_SIZE-CELL_HEIGHT; y += CELL_HEIGHT) {
		drawline(BORDER, y, SCREEN_SIZE-BORDER, y, 1, 0xb77b52);
	}
	// draw equations
	for (int i=0; i<buf_len(lines); ++i) {
		int x = LAST_COL_END;
		int y = DIVIDER_MARGIN + TEXT_OFF + (i * CELL_HEIGHT);
		drawstringf(x, y, FONT_SIZE, BLACK, "%s", lines[i]);
	}
}

void draw_col_highlight(int col, int num_cols, int num_rows) {
	int x = LAST_COL_END + (num_cols - 1 - col) * CELL_WIDTH;
	int y1 = DIVIDER_MARGIN;
	int y2 = DIVIDER_MARGIN + (num_rows * CELL_HEIGHT);
	drawline(x, y1, x, y2, 1, GREEN);
	drawline(x+CELL_WIDTH, y1, x+CELL_WIDTH, y2, 1, GREEN);
}

void draw_markings(BUF(Marking *markings)) {
	for (int j=0; j<buf_len(markings); ++j) {
		Marking m = markings[j];
		drawstringf(m.x, m.y, FONT_SIZE, m.color, "%s", m.text);
	}
}

// NOTE(visualization is meant only for input_small.txt not full puzzle
void visualize(BUF(char **lines), BUF(Equation *equations)) {
	setupgif(1, 1, "trash.gif");

	S64 total = sum(equations);
	int num_cols = (int)strlen(lines[0]);
	int num_rows = buf_len(lines);
	int num_spacing = 3 * CELL_WIDTH + 85;
	int show_work_start_y = DIVIDER_MARGIN + TEXT_OFF + (num_rows+1) * CELL_HEIGHT;
	int show_work_start_x = SCREEN_SIZE - BORDER - DIVIDER_MARGIN - 120;
	int op_spacing = num_spacing + 10;
	int eq_sol_x = BORDER + 75;
	int num_equations = buf_len(equations);
	int eq_i = num_equations - 1;
	int num_i = 0;
	BUF(Marking *markings) = NULL;

	draw_bg(lines);
	for (int i=0; i<PAUSE_FRAMES; ++i) nextframe();

	for (int i=0; i<=num_cols; ++i) {
		int y = show_work_start_y + ((num_equations - 1 - eq_i) * CELL_HEIGHT);

		if ((i+1) % 4 == 0) {
			// render eq solutions
			char *text = xcalloc(16, 1);
			sprintf(text, "%7lld  =", equations[eq_i].solution);
			Marking m = {
				.x = eq_sol_x,
				.y = y,
				.color = BLACK,
				.text = text,
			};
			buf_push(markings, m);
			eq_i -= 1;
			num_i = 0;
		} else {
			// render eq numbers
			S64 num = equations[eq_i].nums[num_i];
			int x = show_work_start_x - (num_i * num_spacing);
			char *text = xcalloc(4, 1);
			sprintf(text, "%3lld", num);
			Marking m = {
				.x = x,
				.y = y,
				.color = BLACK,
				.text = text,
			};
			buf_push(markings, m);

			// render operators
			if ((i+2) % 4 == 0) {
				for (int j=0; j<2; ++j) {
					int x = show_work_start_x - CELL_WIDTH - 25 - (j * op_spacing);
					Marking m = {
						.x = x,
						.y = y,
						.color = BLACK,
						.text = str_from_op(equations[eq_i].op),
					};
					buf_push(markings, m);
				}
			}
			num_i += 1;
		}

		clear();
		draw_bg(lines);
		draw_col_highlight(i, num_cols, num_rows);
		draw_markings(markings);
		for (int j=0; j<PAUSE_FRAMES; ++j) nextframe();
	}

	clear();
	draw_bg(lines);
	draw_markings(markings);
	for (int i=0; i<6; ++i) nextframe();

	// render plus signs
	for (int i=0; i<3; ++i) {
		int x = eq_sol_x - (2 * CELL_WIDTH);
		int y = show_work_start_y + ((i+1) * CELL_HEIGHT);
		drawstringf(x, y, FONT_SIZE, GREEN, "+");
		nextframe();
	}

	// render total sum
	{
		int x = eq_sol_x;
		int y = show_work_start_y + (num_equations * CELL_HEIGHT);
		drawline(x-45, y-8, x+190, y-8, 3, BLACK);
		drawstringf(x, y, FONT_SIZE, GREEN, "%lld", total);
	}

	for (int i=0; i<10; ++i) nextframe();
	drawstringf(500, 760, 12, 0x1212cc, "A+");
	for (int i=0; i<8; ++i) nextframe();

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

	char *file_data_copy = xmalloc(file_size);
	memcpy(file_data_copy, file_data, file_size);

	BUF(Equation *equations) = parse_equations(file_data);
	BUF(Equation *ceph_equations) = parse_ceph_equations(file_data_copy);

	part_one(equations);
	part_two(ceph_equations);

	// NOTE(visualization is meant only for input_small.txt not full puzzle
	// get lines from file_data
	BUF(char **lines) = NULL;
	for (;;) {
		char *line = chop_by_delimiter(&file_data_copy, LINE_END);
		if (*line == '\r' || *line == 0) break;
		for (int i=(int)strlen(line)-1; line[i] == '\r' && i>=0; --i) {
			line[i] = 0;
		}
		buf_push(lines, line);
	}
	visualize(lines, ceph_equations);

	return 0;
}
