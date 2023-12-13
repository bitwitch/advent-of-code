#include <math.h>
#include "../common.c"

typedef enum {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_COUNT,
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
	i32 int_val;
	Color color;
} Token;

typedef struct {
	i32 r, g, b;
} ColorCounts;

typedef enum {
	STATE_NONE,
	STATE_PULL_CUBES,
	STATE_RETURN_CUBES,
	STATE_COUNTDOWN,
} State;

typedef struct {
	oc_vec2 pos, target_pos;
	f32 w, h;
	oc_color color;
} Entity;

typedef struct {
	f64 timer;
	State next_state;
} Countdown;

typedef struct {
	oc_str8 string;
	oc_color color;
	oc_vec2 pos;
} ResultString;

static oc_surface surface;
static oc_canvas canvas;
static oc_font font;
static f32 font_size = 16;
static oc_color bg_color = {0, 0, 0, 1};
static oc_vec2 view_size = {512, 512};
static char *stream;
static Token token;
static State state;
static Entity bag;
static Entity cubes[64];
static i32 num_cubes;
static Countdown countdown;
static f64 dt, last_timestamp;
static i32 current_game;
static ResultString result_strings[100];
static i32 num_result_strings;
static oc_arena *string_arena;

static oc_color cube_colors[COLOR_COUNT] = {
	[COLOR_RED]   = { 1, 0, 0, 1},
	[COLOR_GREEN] = { 0, 1, 0, 1},
	[COLOR_BLUE]  = { 0, 0, 1, 1},
};

static i32 str_n_cmp(const char *_l, const char *_r, size_t n) {
	const unsigned char *l=(void *)_l, *r=(void *)_r;
	if (!n--) return 0;
	for (; *l && *r && n && *l == *r ; l++, r++, n--);
	return *l - *r;
}

static bool is_digit(int c) {
	return c >= 48 && c <= 57;
}

static bool is_alpha(char c) {
	return ((unsigned)c|32)-'a' < 26;
}

static bool is_space(char c) {
	return c == ' ' || (unsigned)c-'\t' < 5;
}

static long str_to_l(char *str, char **out_end, int base) {
	// TODO(shaw): handle bases other than 10
	assert(base == 10);

	while (is_space(*str)) ++str;

	int sign = 1;
	if (*str == '-') {
		++str;
		sign = -1;
	}

	char *start = str;
	while (is_digit(*str)) {
		++str;
	}
	char *end = str;
	int digit_count = end - start;
	int modifier = 1;
	int result = 0;
	for (int i=digit_count-1; i >= 0; --i) {
		int digit = start[i] - '0';
		result += digit * modifier;
		modifier *= 10;
	}

	if (out_end != NULL) {
		*out_end = end;
	}

	return sign * result;
}

static void next_token(void) {
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
	} else if (str_n_cmp(stream, "Game", 4) == 0) {
		token.kind = TOKEN_GAME;
		stream += 4;
	} else if (is_digit(*stream)) {
		token.kind = TOKEN_NUMBER;
		token.int_val = str_to_l(stream, &stream, 10); 
	} else if (is_alpha(*stream)) {
		token.kind = TOKEN_COLOR;
		if (str_n_cmp(stream, "red" , 3) == 0) {
			token.color = COLOR_RED;
			stream += 3;
		} else if (str_n_cmp(stream, "green" , 5) == 0) {
			token.color = COLOR_GREEN;
			stream += 5;
		} else if (str_n_cmp(stream, "blue" , 4) == 0) {
			token.color = COLOR_BLUE;
			stream += 4;
		} else {
			abort("unrecognized color %.*s", 12, stream);
	}
	} else {
		abort("unrecognized token kind");
	}
}

static void init_stream(char *string) {
	stream = string;
	next_token();
}

static void expect_token(TokenKind kind) {
	if (token.kind != kind) {
		abort("unexpected token");
	}
	next_token();
}

static bool match_token(TokenKind kind) {
	if (token.kind == kind) {
		next_token();
		return true;
	}
	return false;
}

static bool is_token(TokenKind kind) {
	return token.kind == kind;
}

static i32 parse_number(void) {
	if (token.kind != TOKEN_NUMBER) {
		abort("expected token number");
	}
	i32 number = token.int_val;
	next_token();
	return number;
}

static Color parse_color(void) {
	if (token.kind != TOKEN_COLOR) {
		abort("expected token color");
	}
	Color color = token.color;
	next_token();
	return color;
}

static i32 parse_game_number(void) {
	expect_token(TOKEN_GAME);
	i32 result = parse_number();
	expect_token(TOKEN_COLON);
	return result;
}

static ColorCounts parse_reveal(void) {
	ColorCounts reveal = {0};
	do {
		i32 count = parse_number();
		Color color = parse_color();
		if (color == COLOR_RED)   reveal.r = count;
		if (color == COLOR_GREEN) reveal.g = count;
		if (color == COLOR_BLUE)  reveal.b = count;
	} while (match_token(TOKEN_COMMA));
	return reveal;
}

static ColorCounts parse_game(void) {
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

static void part_one(char *file_data, u64 file_size) {
	(void)file_size;

	init_stream(file_data);

	u64 sum = 0;
	do {
		i32 game_num = parse_game_number();
		ColorCounts maxes = parse_game();
		if (maxes.r <= 12 && maxes.g <= 13 && maxes.b <= 14) {
			sum += game_num;
		}
	} while (!is_token(TOKEN_EOF));

	oc_log_info("Part one: %zu\n", sum);
}

static void part_two(char *file_data, u64 file_size) {
	(void)file_size;

	init_stream(file_data);

	u64 sum = 0;
	do {
		parse_game_number();
		ColorCounts maxes = parse_game();
		i32 power = maxes.r * maxes.g * maxes.b;
		sum += power;
	} while (!is_token(TOKEN_EOF));

	oc_log_info("Part two: %zu\n", sum);
}

static void test_str_to_l(void) {
	long a = str_to_l("12345", NULL, 10);
	assert(a == 12345);

	char *str_b = "666 farts";
	char *end_b = NULL;
	long b = str_to_l(str_b, &end_b, 10);
	assert(b == 666);
	assert(end_b == str_b + 3);

	long c = str_to_l("   420 dude", NULL, 10);
	assert(c == 420);

	long d = str_to_l("  -69   ", NULL, 10);
	assert(d == -69);

	oc_log_info("Success");
}

static void calculate_next_pull(void) {
	if (is_token(TOKEN_NEWLINE)) {
		next_token();
	}
	if (is_token(TOKEN_GAME)) {
		current_game = parse_game_number();

		i32 max_rows = 28;
		i32 col = (current_game-1) / max_rows;
		i32 col_width = 2 * font_size - 8;
		i32 row = (current_game-1) % max_rows;
		i32 row_height = font_size + 2;

		result_strings[num_result_strings++] = (ResultString) {
			.string = oc_str8_pushf(string_arena, "%d", current_game),
			.color = { 255, 255, 255, 1 },
			.pos = { 5 + col * col_width, 3 + font_size + row * row_height }
		};
	}
	if (match_token(TOKEN_SEMICOLON)) {
		// update maxes
	} 

	num_cubes = 0;
	ColorCounts reveal = parse_reveal();
	for (i32 i=0; i<reveal.r; ++i) {
		assert(num_cubes < ARRAY_COUNT(cubes));
		Entity *cube = &cubes[num_cubes++];
		cube->color = cube_colors[COLOR_RED];
		cube->target_pos.x = rand_range_u32(bag.pos.x, bag.pos.x + bag.w);
		cube->target_pos.y = rand_range_u32(50, bag.pos.y - 50);
	}
	for (i32 i=0; i<reveal.g; ++i) {
		assert(num_cubes < ARRAY_COUNT(cubes));
		Entity *cube = &cubes[num_cubes++];
		cube->color = cube_colors[COLOR_GREEN];
		cube->target_pos.x = rand_range_u32(bag.pos.x, bag.pos.x + bag.w),
		cube->target_pos.y = rand_range_u32(50, bag.pos.y - 50);
	}
	for (i32 i=0; i<reveal.b; ++i) {
		assert(num_cubes < ARRAY_COUNT(cubes));
		Entity *cube = &cubes[num_cubes++];
		cube->color = cube_colors[COLOR_BLUE];
		cube->target_pos.x = rand_range_u32(bag.pos.x, bag.pos.x + bag.w);
		cube->target_pos.y = rand_range_u32(50, bag.pos.y - 50);
	}
}

static f32 vec2_dist(oc_vec2 v1, oc_vec2 v2) {
	f32 a = v2.x - v1.x;
	f32 b = v2.y - v1.y;
	return sqrtf(a*a + b*b);
}

static bool all_cubes_at_target(void) {
	for (i32 i=0; i<num_cubes; ++i) {
		if (vec2_dist(cubes[i].pos, cubes[i].target_pos) > 2) {
			return false;
		}
	}
	return true;
}

static void step_cubes(f32 rate) {
	for (i32 i=0; i<num_cubes; ++i) {
		Entity *cube = &cubes[i];
		if (cube->pos.x != cube->target_pos.x || cube->pos.y != cube->target_pos.y) {
			if (vec2_dist(cube->pos, cube->target_pos) < 2) {
				cube->pos = cube->target_pos;
			} else {
				cube->pos.x += (cube->target_pos.x - cube->pos.x) * rate * dt;
				cube->pos.y += (cube->target_pos.y - cube->pos.y) * rate * dt;
			}
		}
	}
}

static void update_pull_cubes(void) {
	step_cubes(100);
	
	if (all_cubes_at_target()) {
		for (i32 i=0; i<num_cubes; ++i) {
			cubes[i].target_pos.x = bag.pos.x + bag.w/2;
			cubes[i].target_pos.y = bag.pos.y + 25;
		}
		state = STATE_COUNTDOWN;
		countdown.timer = 0;
		countdown.next_state = STATE_RETURN_CUBES;
	}
}

static void update_return_cubes(void) {
	step_cubes(100);

	// check for state transition
	if (all_cubes_at_target()) {
		calculate_next_pull();
		state = STATE_PULL_CUBES;
		// state = STATE_COUNTDOWN;
		// countdown.timer = 0;
		// countdown.next_state = STATE_PULL_CUBES;
	}
}

static void update_countdown(void) {
	if (countdown.timer > 0) {
		countdown.timer -= dt;
	} else {
		state = countdown.next_state;
	}
}

static void draw(void) {
    oc_canvas_select(canvas);
	oc_surface_select(surface);
	oc_set_color(bg_color);
	oc_clear();

	// draw back of bag

	// draw cubes	
	for (i32 i=0; i<num_cubes; ++i) {
		oc_set_color(cubes[i].color);
		oc_rectangle_fill(cubes[i].pos.x, cubes[i].pos.y, cubes[i].w, cubes[i].h);
	}
	
	// draw front of bag
	oc_set_color(bag.color);
	oc_rectangle_fill(bag.pos.x, bag.pos.y, bag.w, bag.h);

	// draw hud
	oc_set_font_size(font_size);
	for (i32 i=0; i<num_result_strings; ++i) {
		ResultString *rs = &result_strings[i];
		oc_set_color(rs->color);
		oc_text_fill(rs->pos.x, rs->pos.y, rs->string);
	}

    oc_render(canvas);
    oc_surface_present(surface);
}

ORCA_EXPORT void oc_on_init(void) {
	// test_str_to_l();

	// init random number generator
	f64 ftime = oc_clock_time(OC_CLOCK_MONOTONIC);
	u64 time = *((u64*)&ftime);
	pcg32_init(time);

	oc_arena_init(string_arena);

    oc_window_set_title(OC_STR8("Cube Conundrum"));
	oc_window_set_size(view_size);

    surface = oc_surface_canvas();
    canvas = oc_canvas_create();

    oc_unicode_range ranges[5] = {
        OC_UNICODE_BASIC_LATIN,
        OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
        OC_UNICODE_LATIN_EXTENDED_A,
        OC_UNICODE_LATIN_EXTENDED_B,
        OC_UNICODE_SPECIALS,
    };
	font = oc_font_create_from_path(OC_STR8("segoeui.ttf"), 5, ranges);
	oc_set_font(font);

	bag.w = view_size.x * 0.75;
	bag.h = view_size.y * 0.45;;
	bag.pos.x = view_size.x/2 - bag.w/2;
	bag.pos.y = view_size.y - bag.h - 50;
	bag.color = (oc_color){115/255.0f, 78/255.0f, 62/255.0f, 1.0f};

	// temporary random cube generation
	for (i32 i=0; i<ARRAY_COUNT(cubes); ++i) {
		Entity *cube = &cubes[i];
		cube->pos.x = bag.pos.x + bag.w/2;
		cube->pos.y = bag.pos.y + 20;
		cube->w = 20;
		cube->h = 20;
	}

	char *filename = "input.txt";
	char *file_data;
	u64 file_size;
	if (!read_entire_file(filename, &file_data, &file_size)) {
		oc_log_error("failed to read file %s\n", filename);
	}

	init_stream(file_data);

	// part_one(file_data, file_size);
	// part_two(file_data, file_size);

    last_timestamp = oc_clock_time(OC_CLOCK_MONOTONIC);

	calculate_next_pull();
	state = STATE_COUNTDOWN;
	countdown.timer = 0.5;
	countdown.next_state = STATE_PULL_CUBES;
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height) {
	view_size.x = width;
	view_size.y = height;
}

ORCA_EXPORT void oc_on_key_down(oc_scan_code scan, oc_key_code key) {

}

ORCA_EXPORT void oc_on_key_up(oc_scan_code scan, oc_key_code key) {

}

// ORCA_EXPORT void oc_on_mouse_down(int button) {
	// if (button == OC_MOUSE_LEFT) {
		// game.mouse_input.left.down = true;
	// } else if (button == OC_MOUSE_RIGHT) {
		// game.mouse_input.right.down = true;
	// }
// }

// ORCA_EXPORT void oc_on_mouse_up(int button) {
	// if (button == OC_MOUSE_LEFT) {
		// game.mouse_input.left.down = false;
	// } else if (button == OC_MOUSE_RIGHT) {
		// game.mouse_input.right.down = false;
	// }
// }

// ORCA_EXPORT void oc_on_mouse_move(float x, float y, float dx, float dy) {
    // game.mouse_input.x = x;
    // game.mouse_input.y = y;
    // game.mouse_input.deltaX = dx;
    // game.mouse_input.deltaY = dy;
// }

ORCA_EXPORT void oc_on_frame_refresh(void) {
    f64 timestamp = oc_clock_time(OC_CLOCK_MONOTONIC);
	dt = timestamp - last_timestamp;
	last_timestamp = timestamp;
	
	switch(state) {
		case STATE_PULL_CUBES:   update_pull_cubes();   break;
		case STATE_RETURN_CUBES: update_return_cubes(); break;
		case STATE_COUNTDOWN:    update_countdown();    break;
		default:                 assert(0);             break;
	}

	draw();
}



