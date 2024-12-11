#include "..\common.c"
#include "..\lex.c"
#include "..\..\base\base_inc.h"
#include "..\..\base\base_inc.c"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

#define RED        0xFF3232AC 
#define DARK_RED   0xFF1D1D8C
#define GREEN       0xFF34A82E
#define TEXT_GREEN 0xFF7FEA8A
#define BLACK      0xFF1D201D
#define PALE_SKIN  0xFF9AC3EE
#define BROWN      0xFF2B4189
#define ORANGE     0xFF3363BC
#define WHITE      0xFFF7F5EA
#define GOLD       0xFF15B4EC

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

void draw_shiny_corner(int x, int y, int orientation) {
	int scale = 3;
	int five = 5*scale;
	int six = 6*scale;
	int two = 2*scale;
	switch (orientation) {
	case 0:
		drawbox(x, y, six, scale, GOLD);
		drawbox(x, y+scale, five, scale, GOLD);
		drawbox(x, y, scale, six, GOLD);
		drawbox(x+scale, y, scale, five, GOLD);
		break;
	case 1:
		drawbox(x-six, y, six, scale, GOLD);
		drawbox(x-five, y+scale, five, scale, GOLD);
		drawbox(x-scale, y, scale, six, GOLD);
		drawbox(x-two, y, scale, five, GOLD);
		break;
	case 2:
		drawbox(x-six, y-scale, six, scale, GOLD);
		drawbox(x-five, y-two, five, scale, GOLD);
		drawbox(x-scale, y-six, scale, six, GOLD);
		drawbox(x-two, y-five, scale, five, GOLD);
		break;
	case 3:
		drawbox(x, y-scale, six, scale, GOLD);
		drawbox(x, y-two, five, scale, GOLD);
		drawbox(x, y-six, scale, six, GOLD);
		drawbox(x+scale, y-five, scale, five, GOLD);
		break;
	default:
		fprintf(stderr, "Error: draw_shiny_corner orientation must be between 0 and 3: got %d\n", orientation);
		break;
	}

}

void draw_bg(void) {
	{ // desk and wall
		int y = 450;
		drawbox(0, 0, SCREEN_WIDTH, y, ORANGE);
		drawbox(0, y, SCREEN_WIDTH, SCREEN_HEIGHT - y, BROWN);
	}
	
	{ // computer
		int comp_width = 600;
		int comp_height = 700;
		int screen_pad = 30;
		int pad = 20;
		drawbox(pad, pad, comp_width, comp_height, RED);
		drawbox(pad+screen_pad, pad+screen_pad, 
			comp_width - 2*screen_pad,
			comp_height - 2*screen_pad, 
			BLACK);

		draw_shiny_corner(pad, pad, 0);
		draw_shiny_corner(pad+comp_width, pad, 1);
	}
	
	{ // memory view
		int pad = 20;
		int screen_pad = 30;
		int mem_width = 349;
		int mem_height = SCREEN_HEIGHT - 2*pad;
		int x = SCREEN_WIDTH - mem_width - pad;
		drawbox(x, pad, mem_width, mem_height, RED);
		drawbox(x+screen_pad, pad+screen_pad, 
			mem_width - 2*screen_pad,
			mem_height - 2*screen_pad, 
			BLACK);

		draw_shiny_corner(x, pad, 0);
		draw_shiny_corner(x+mem_width, pad, 1);
		draw_shiny_corner(x+mem_width, pad+mem_height, 2);
		draw_shiny_corner(x, pad+mem_height, 3);
	}

	{ // keyboard
		int y = 780;
		int key_width = 300;
		int key_height = 150;
		drawbox(0, y, key_width, key_height, RED);
		drawtri(key_width, y + key_height,
		        key_width, y,
				key_width + 20, y + key_height, 
				RED);
		drawbox(0, y + key_height, key_width + 20, 20, DARK_RED);
		drawline(230, y, 220, 720, 3, BLACK); // cord
	}

	{ // mouse
		int x = 380;
		int y = 850;
		int h = 25;
		int w = 50;
		int skew_x = 13;
		int skew_y = 60;
		drawbox(x, y, w, h, RED);
		drawtri(x, y+h,
				x+skew_x+w, y+skew_y-h,
				x+skew_x, y+skew_y,
				RED);
		drawtri(x+w, y,
				x+skew_x+w-1, y+skew_y-h,
				x+skew_x, y+skew_y,
				RED);
		drawtri(x, y+h,
				x+w, y,
				x+w, y+skew_y,
				RED);
		drawbox(x+skew_x, y+skew_y-h, w, h, DARK_RED);
		drawline(x+(w/2.0f), y, 320, 720, 3, BLACK); //cord
	}

	{ // elf
		int x = 160;
		int y = 810;
		int r = 55;
		int poof_x = x - 12;
		int poof_y = y - 132;
		drawcircle(x, y, r, PALE_SKIN);                 // face
		drawtri(
			x - r - 4, y + 3, 
			poof_x, poof_y,                             // hat
			x + r + 2, y - 12, 
			GREEN);                                   
		drawcircle(poof_x, poof_y, 12, WHITE);          // poof
		drawbox(x - r, y + r, 100, 100, GREEN);          // body
		drawbox(x - r + 20, y + r + 100, 90, 30, GREEN); // legs
		drawbox(x + r - 17, y + r + 13, 20, 43, GREEN);  // arm
	}
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

void draw_memory(char *text) {
#define NUM_COLS 16
	char row[NUM_COLS+1] = {0};
	int x = SCREEN_WIDTH - 322;
	int y = 68;
	int font_size = 2;
	U64 num_rows = MIN(strlen(text) / NUM_COLS, 37);

	for (int j=0; j<num_rows; ++j) {
		int i = j*NUM_COLS;
		strncpy(row, text+i, NUM_COLS);
		drawstringn(row, x, y, font_size, TEXT_GREEN);
		y += 12 * font_size;
	}

#undef NUM_COLS 
}

void draw_computer(void) {
	char *tmp = "mul(2,4)x?!xmultmul";
	int start_x = 65;
	int start_y = 65;
	int x = start_x;
	int y = start_y;
	int font_size = 3;
	drawstringn(tmp, x, y, font_size, TEXT_GREEN);

}

void visualize(char *file_name, char *file_data) {
    setupgif(0, 1, "mull.gif");

	clear();
	draw_bg();
	nextframe();

	init_lexer(file_name, file_data);
	S64 result = 0;
	bool enabled = true;

	U64 frames = 0;
	while (frames < 50) {
		// while (!is_token(TOKEN_EOF)) {
			// if (is_mul()) {
				// Mul mul = parse_mul();
				// if (enabled) result += mul.result;
			// } else if (match_do()) {
				// enabled = true;
			// } else if (match_dont()) {
				// enabled = false;
			// } else {
				// _stream = _token.start + 1;
				// next_token();
			// }
		// }

		clear();
		draw_bg();
		draw_memory(_stream);
		draw_computer();

		++frames;
		nextframe();
	}
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


	part_one(file_name, file_data);
	part_two(file_name, file_data);
	visualize(file_name, file_data);

	return 0;
}
