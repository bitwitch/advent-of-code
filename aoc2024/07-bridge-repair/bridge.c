#include <math.h>   // for pow()
#include "../common.c"
#include "../lex.c"

typedef struct {
	S64 expected;
	BUF(S64 *nums);
} Equation;

typedef enum {
	OP_ADD,
	OP_MUL,
	OP_CONCAT,
	COUNT_OPS,
} Op;

S64 op_execute_concat(S64 lhs, S64 rhs) {
	int digits = 0;
	S64 rhs_copy = rhs;
	while (rhs_copy > 0) {
		++digits;
		rhs_copy /= 10;
	}
	if (rhs == 0) digits = 1;
	S64 result = (lhs * (S64)pow(10, digits)) + rhs;
	return result;
}

S64 op_execute(Op op, S64 lhs, S64 rhs) {
	switch (op) {
		case OP_ADD:    return lhs + rhs;
		case OP_MUL:    return lhs * rhs;
		case OP_CONCAT: return op_execute_concat(lhs, rhs);
		default: 
			fprintf(stderr, "Unrecognized op in op_execute: %d\n", op);
			exit(1);
	}
}

bool eq_valid_recur(Equation e, S64 lhs, Op op, S64 rhs_index) {
	if (rhs_index == buf_len(e.nums)) {
		return lhs == e.expected;
	}

	S64 new_lhs = op_execute(op, lhs, e.nums[rhs_index]);

	return eq_valid_recur(e, new_lhs, OP_ADD, rhs_index + 1) ||
	       eq_valid_recur(e, new_lhs, OP_MUL, rhs_index + 1);
}

bool eq_valid(Equation e) {
	return eq_valid_recur(e, 0, OP_ADD, 0) ||
	       eq_valid_recur(e, 1, OP_MUL, 0);
}

void part_one(BUF(Equation *eqs)) {
	S64 result = 0;
	for (S64 j=0; j<buf_len(eqs); ++j) {
		if (eq_valid(eqs[j])) {
			result += eqs[j].expected;
		}
	}
	printf("part one: %lld\n", result);
}

bool eq_valid_recur_2(Equation e, S64 lhs, Op op, S64 rhs_index) {
	if (rhs_index == buf_len(e.nums)) {
		return lhs == e.expected;
	}
	S64 new_lhs = op_execute(op, lhs, e.nums[rhs_index]);
	for (Op op=0; op<COUNT_OPS; ++op) {
		if (eq_valid_recur_2(e, new_lhs, op, rhs_index + 1)) {
			return true;
		}
	}
	return false;
}

bool eq_valid_2(Equation e) {
	for (Op op=0; op<COUNT_OPS; ++op) {
		if (eq_valid_recur_2(e, e.nums[0], op, 1)) {
			return true;
		}
	}
	return false;
}

void eq_print(Equation e) {
	printf("%lld:", e.expected);
	for (int i=0; i<buf_len(e.nums); ++i) {
		printf(" %lld", e.nums[i]);
	}
}

void part_two(BUF(Equation *eqs)) {
	S64 result = 0;
	for (S64 j=0; j<buf_len(eqs); ++j) {
		if (eq_valid_2(eqs[j])) {
			result += eqs[j].expected;
		} 
	}
	printf("part two: %lld\n", result);
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

	BUF(Equation *eqs) = NULL;
	while (!is_token(TOKEN_EOF)) {
		Equation e = {0};
		e.expected = parse_int();
		expect_token(':');
		while (is_token(TOKEN_INT)) {
			buf_push(e.nums, (S64)parse_int());
		}
		buf_push(eqs, e);
		while (is_token('\n')) next_token();
	}

	part_one(eqs);
	part_two(eqs);

	return 0;
}

