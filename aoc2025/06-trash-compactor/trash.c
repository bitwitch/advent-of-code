#include "../common.c"

#if _WIN32
#define LINE_END "\r\n"
#else
#define LINE_END "\n"
#endif

typedef enum {
	OP_NONE,
	OP_ADD,
	OP_MUL,
} Op;

typedef struct {
	Op op;
	BUF(S64 *nums); 
} Equation;

Op op_from_char(char c) {
	switch (c) {
		case '+': return OP_ADD;
		case '*': return OP_MUL;
		default:  return OP_NONE;
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

	return 0;
}
