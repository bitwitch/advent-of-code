#include "../common.c"

typedef enum {
	OP_NONE,
	OP_ADD,
	OP_MUL,
} Op;

typedef struct {
	BUF(S64 *nums); 
	Op op;
} Equation;

Op op_from_char(char c) {
	switch (c) {
		case '+': return OP_ADD;
		case '*': return OP_MUL;
		default:  return OP_NONE;
	}
}

void part_one(BUF(Equation *equations)) {
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

	printf("part one: %llu\n", result);
}

void part_two(BUF(Equation *equations)) {
	U64 result = 0;
	// printf("part two: %llu\n", result);
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


	BUF(Equation *equations) = parse_equations(file_data);

	part_one(equations);
	part_two(equations);

	return 0;
}
