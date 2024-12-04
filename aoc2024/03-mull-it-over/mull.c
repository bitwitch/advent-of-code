#include "..\common.c"
#include "..\lex.c"

typedef struct {
		Token token;
		char *stream;
		int current_line;
} LexState;

LexState get_lex_state(void) {
	return (LexState) {
		.token = _token,
		.stream = _stream,
		.current_line = _current_line
	};
}

void set_lex_state(LexState state) {
	_token = state.token;
	_stream = state.stream;
	_current_line = state.current_line;
}

typedef struct {
	S64 op1, op2, result;
} Mul;

bool is_mul(void) {
	bool result = false;
	LexState start_state = get_lex_state();
	if (match_token_name("mul")        && 
		match_token('(')       && 
		match_token(TOKEN_INT) && 
		match_token(',')       && 
		match_token(TOKEN_INT) && 
		match_token(')'))
	{
		result = true;
	}
	set_lex_state(start_state);
	return result;
}

Mul parse_mul(void) {
	Mul mul = {0};
	if (!match_token_name("mul")) {
		fatal("Expected mul, got %s", token_kind_to_str(_token.kind));
	}
	expect_token('(');

	if (!is_token(TOKEN_INT)) {
		fatal("Expected int, got %s", token_kind_to_str(_token.kind));
	}
	mul.op1 = (S64)parse_int();

	expect_token(',');

	if (!is_token(TOKEN_INT)) {
		fatal("Expected int, got %s", token_kind_to_str(_token.kind));
	}
	mul.op2 = (S64)parse_int();

	mul.result = mul.op1 * mul.op2;

	expect_token(')');

	return mul;
}

bool match_do(void) {
	return match_token_name("do") && match_token('(') && match_token(')');
}

bool match_dont(void) {
	return match_token_name("don") && match_token('\'') && match_token_name("t") &&
		   match_token('(') && match_token(')');
}

void part_one(char *file_name, char *file_data) {
	init_lexer(file_name, file_data);
	S64 result = 0;
	while (!is_token(TOKEN_EOF)) {
		if (match_token_name("mul")) {
			if (match_token('(')) {
				if (is_token(TOKEN_INT)) {
					int i1 = (int)parse_int();
					if (match_token(',')) {
						if (is_token(TOKEN_INT)) {
							int i2 = (int)parse_int();
							if (match_token(')')) {
								S64 product = i1 * i2;
								result += product;
								// printf("mul(%d,%d)=%lld sum=%lld\n", i1, i2, product, result);
								continue;
							}
						}
					}
				}
			}
		} 
		_stream = _token.start + 1;
		next_token();
	}

	printf("part one: %lld\n", result);
}


void part_two(char *file_name, char *file_data) {
	init_lexer(file_name, file_data);
	S64 result = 0;
	bool enabled = true;
	while (!is_token(TOKEN_EOF)) {
		if (is_mul()) {
			Mul mul = parse_mul();
			if (enabled) result += mul.result;
		} else if (match_do()) {
			enabled = true;
		} else if (match_dont()) {
			enabled = false;
		} else {
			_stream = _token.start + 1;
			next_token();
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

	part_one(file_name, file_data);
	part_two(file_name, file_data);

	return 0;
}
