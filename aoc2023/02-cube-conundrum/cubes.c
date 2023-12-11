#include "..\common.c"
#include <ctype.h>

typedef enum {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
} Color; 

typedef enum {
	TOKEN_NONE,
	TOKEN_GAME,
	TOKEN_NUMBER,
	TOKEN_COLOR,
	TOKEN_COMMA,
	TOKEN_COLON,
	TOKEN_SEMICOLON,
	TOKEN_NEWLINE,
	TOKEN_EOF,
} TokenKind;

typedef struct {
	TokenKind kind;
	int int_val;
	Color color;
} Token;

typedef struct {
	int r, g, b;
} ColorCounts;

static char *stream;
static Token token;

void next_token(void) {
	// eat spaces
	while (*stream == ' ' || *stream == '\r' || *stream == '\t') {
		++stream;
	}

	if (*stream == ':') {
		token.kind = TOKEN_COLON;
		++stream;
	} else if (*stream == ';') {
		token.kind = TOKEN_SEMICOLON;
		++stream;
	} else if (*stream == ',') {
		token.kind = TOKEN_COMMA;
		++stream;
	} else if (*stream == '\n') {
		token.kind = TOKEN_NEWLINE;
		++stream;
	} else if (*stream == '\0') {
		token.kind = TOKEN_EOF;
		++stream;
	} else if (strncmp(stream, "Game", 4) == 0) {
		token.kind = TOKEN_GAME;
		stream += 4;
	} else if (isdigit(*stream)) {
		token.kind = TOKEN_NUMBER;
		token.int_val = strtol(stream, &stream, 10); 
	} else if (isalpha(*stream)) {
		token.kind = TOKEN_COLOR;
		if (strncmp(stream, "red" , 3) == 0) {
			token.color = COLOR_RED;
			stream += 3;
		} else if (strncmp(stream, "green" , 5) == 0) {
			token.color = COLOR_GREEN;
			stream += 5;
		} else if (strncmp(stream, "blue" , 4) == 0) {
			token.color = COLOR_BLUE;
			stream += 4;
		} else {
			fatal("unrecognized color");
		}
	} else {
		fatal("unrecognized token kind");
	}
}

void init_stream(char *string) {
	stream = string;
	next_token();
}

void expect_token(TokenKind kind) {
	if (token.kind != kind) {
		fatal("unexpected token");
	}
	next_token();
}

bool match_token(TokenKind kind) {
	if (token.kind == kind) {
		next_token();
		return true;
	}
	return false;
}

bool is_token(TokenKind kind) {
	return token.kind == kind;
}

int parse_number(void) {
	if (token.kind != TOKEN_NUMBER) {
		fatal("expected token number");
	}
	int number = token.int_val;
	next_token();
	return number;
}

Color parse_color(void) {
	if (token.kind != TOKEN_COLOR) {
		fatal("expected token color");
	}
	Color color = token.color;
	next_token();
	return color;
}

int parse_game_number(void) {
	expect_token(TOKEN_GAME);
	int result = parse_number();
	expect_token(TOKEN_COLON);
	return result;
}

ColorCounts parse_reveal(void) {
	ColorCounts reveal = {0};
	do {
		int count = parse_number();
		Color color = parse_color();
		if (color == COLOR_RED)   reveal.r = count;
		if (color == COLOR_GREEN) reveal.g = count;
		if (color == COLOR_BLUE)  reveal.b = count;
	} while (match_token(TOKEN_COMMA));
	return reveal;
}

ColorCounts parse_game(void) {
	ColorCounts maxes = {0};
	do {
		ColorCounts reveal = parse_reveal();
		if (reveal.r > maxes.r) maxes.r = reveal.r;
		if (reveal.g > maxes.g) maxes.g = reveal.g;
		if (reveal.b > maxes.b) maxes.b = reveal.b;
	} while (match_token(TOKEN_SEMICOLON));
	if (is_token(TOKEN_NEWLINE)) {
		next_token();
	}
	return maxes;
}

void part_one(char *file_data, U64 file_size) {
	(void)file_size;

	init_stream(file_data);

	U64 sum = 0;
	do {
		int game_num = parse_game_number();
		ColorCounts maxes = parse_game();
		if (maxes.r <= 12 && maxes.g <= 13 && maxes.b <= 14) {
			sum += game_num;
		}
	} while (!is_token(TOKEN_EOF));

	printf("Part one: %zu\n", sum);
}

void part_two(char *file_data, U64 file_size) {
	(void)file_size;

	init_stream(file_data);

	U64 sum = 0;
	do {
		parse_game_number();
		ColorCounts maxes = parse_game();
		int power = maxes.r * maxes.g * maxes.b;
		sum += power;
	} while (!is_token(TOKEN_EOF));

	printf("Part two: %zu\n", sum);
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

	part_one(file_data, file_size);
	part_two(file_data, file_size);

	return 0;
}
