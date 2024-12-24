#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define RED             0xFF3232AC 
#define DARK_RED        0xFF1D1D8C
#define GREEN           0xFF34A82E
#define TEXT_GREEN      0xFF7FEA8A
#define BLACK           0xFF1D201D
#define PALE_SKIN       0xFF9AC3EE
#define BROWN           0xFF2B4189
#define DARK_BROWN      0xFF041952 
#define ORANGE          0xFF3363BC
#define WHITE           0xFFF7F5EA
#define GOLD            0xFF15B4EC

#define GREEN_LIGHT_ON_HIGHLIGHT  0xFFF8FEDC
#define GREEN_LIGHT_ON            0xFF2EFC00
#define GREEN_LIGHT_OFF           0xFF445E47
#define RED_LIGHT_ON_HIGHLIGHT    0xFF03EDFF
#define RED_LIGHT_ON              0xFF0100EA
#define RED_LIGHT_OFF             0xFF313169 

#define SCREEN_WIDTH      1024
#define SCREEN_HEIGHT     1024
#define COMP_TOP_X        65
#define COMP_TOP_Y        65
#define COMP_FONT_SIZE    3

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
		int pad = 20;
		int width = 600;
		int top_height = 132;
		int bot_pad = 15;
		int bot_height = 700 - top_height - bot_pad;
		int bot_y = pad + top_height+bot_pad;
		int top_screen_pad = 30;
		int bot_screen_pad = 30;

		drawbox(pad, pad, width, top_height, RED);
		drawbox(pad+top_screen_pad, pad+top_screen_pad, 
			width - 2*top_screen_pad,
			top_height - 2*top_screen_pad, 
			BLACK);
		draw_shiny_corner(pad, pad, 0);
		draw_shiny_corner(pad+width, pad, 1);
		draw_shiny_corner(pad+width, pad+top_height, 2);
		draw_shiny_corner(pad, pad+top_height, 3);

		drawbox(pad, bot_y, width, bot_height, RED);
		drawbox(pad+bot_screen_pad, bot_y+bot_screen_pad, 
			width - 2*bot_screen_pad,
			bot_height - 2*bot_screen_pad, 
			BLACK);
		draw_shiny_corner(pad, bot_y, 0);
		draw_shiny_corner(pad+width, bot_y, 1);
		draw_shiny_corner(pad+width, bot_y+bot_height, 2);
		draw_shiny_corner(pad, bot_y+bot_height, 3);

		int feet_width = 55;
		int feet_space = 40;
		drawbox(pad+feet_space, pad+top_height, feet_width, 15, DARK_BROWN);
		drawbox(pad+width-feet_width-feet_space, pad+top_height, feet_width, 15, DARK_BROWN);

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

typedef struct {
	char *stream;
	U64 sum;
	bool mul_enabled;
	bool highlight_current;
	int highlight_count; // num chars in expression to highlight
	char **lines;
	int num_lines;
} Display;

void draw_computer(Display *display) {
	// draw input view in top computer screen
	int max_cols = 21;
	drawstringf(COMP_TOP_X, COMP_TOP_Y, COMP_FONT_SIZE, TEXT_GREEN, "%.*s", max_cols, display->stream);

	if (display->highlight_current) {
		drawstringf(COMP_TOP_X, COMP_TOP_Y, COMP_FONT_SIZE, GOLD, "%.*s", display->highlight_count, display->stream);
	}

	// draw enabled/disabled lights
	int light_x = COMP_TOP_X + 10;
	int light_y = COMP_TOP_Y + 72;
	int light_r = 12;
	int off_x = 2 * light_r + 10;
	drawcircle(light_x, light_y, light_r, BLACK);
	drawcircle(light_x + off_x, light_y, light_r, BLACK);
	light_r = 9;
	if (display->mul_enabled) {
		drawcircle(light_x, light_y, light_r, GREEN_LIGHT_ON);
		drawcircle(light_x, light_y, 4, GREEN_LIGHT_ON_HIGHLIGHT);
		drawcircle(light_x + off_x, light_y, light_r, RED_LIGHT_OFF);
	} else {
		drawcircle(light_x, light_y, light_r, GREEN_LIGHT_OFF);
		drawcircle(light_x + off_x, light_y, light_r, RED_LIGHT_ON);
		drawcircle(light_x + off_x, light_y, 4, RED_LIGHT_ON_HIGHLIGHT);
	}

	// draw mul lines
	int x = 65;
	int start_y = 215;
	int last = display->num_lines;
	int max_lines = last < 10 ? last : 10;
	for (int i=0; i<max_lines; ++i) {
		int row = max_lines - i - 1;
		int y = start_y + row*12*COMP_FONT_SIZE;
		drawstringf(x, y, COMP_FONT_SIZE, TEXT_GREEN, "%.*s", max_cols, display->lines[last-1-i]);
	}

	// draw sum
	int sum_digits = 8;
	drawstringf(224, 636, COMP_FONT_SIZE, TEXT_GREEN, "sum = %*d", sum_digits, display->sum);
}

void visualize(char *file_name, char *file_data) {
    setupgif(0, 1, "mull.gif");
	init_lexer(file_name, file_data);

	Display display = {0};
	display.stream = file_data;
	display.sum = 0;
	display.mul_enabled = true;

	clear();
	draw_bg();
	nextframe();

	U64 frames = 0;
	U64 skip_frames = 1;

	while (!is_token(TOKEN_EOF)) {
		LexState start_state = get_lex_state();
		U64 freeze_frames = 0;
		display.highlight_current = false;
		if (is_mul()) {
			Mul mul = parse_mul();
			if (display.mul_enabled) {
				char *line = xmalloc(64 * sizeof(char));
				snprintf(line, 64, "mul(%3lld,%3lld) =%7lld", mul.op1, mul.op2, mul.result);
				buf_push(display.lines, line);
				display.num_lines = buf_len(display.lines);
				display.sum += mul.result;
				display.highlight_current = true;
				display.highlight_count = (int)(_token.start - start_state.token.start);
				freeze_frames = 2;
			}
		} else if (match_do()) {
			display.mul_enabled = true;
			display.highlight_current = true;
			display.highlight_count = (int)(_token.start - start_state.token.start);
			freeze_frames = 2;
		} else if (match_dont()) {
			display.mul_enabled = false;
			display.highlight_current = true;
			display.highlight_count = (int)(_token.start - start_state.token.start);
			freeze_frames = 2;
		} else {
			start_state.stream = start_state.token.start + 1;
			set_lex_state(start_state);
			next_token();
		}

		display.stream = start_state.token.start;

		switch (frames) {
			case 100:   skip_frames = 6;   break;
			case 520:   skip_frames = 24;  break;
			case 1400:  skip_frames = 200; break;
			case 4000:  skip_frames = 250; break;
			case 10900: skip_frames = 6;   break;
			case 11050: skip_frames = 1;   break;
			default: break;
		}

		if (frames > 520 && frames < 10900) {
			freeze_frames = 0;
		}

		if ((frames++ % skip_frames) == 0) {
			clear();
			draw_bg();
			draw_memory(_stream);
			draw_computer(&display);
			nextframe();
			// do freeze frames
			for (; freeze_frames > 0; --freeze_frames) nextframe();
		}
	}

	clear();
	draw_bg();
	draw_memory(_stream);
	draw_computer(&display);
	for (int i=0; i<20; ++i) nextframe();

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
