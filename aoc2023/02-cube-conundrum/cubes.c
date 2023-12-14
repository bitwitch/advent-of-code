#include <math.h>
#include "../common.c"

typedef enum {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_WHITE,
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
	STATE_ADDITION,
	STATE_DONE,
} State;

typedef struct {
	oc_vec2 pos, target_pos;
	f32 w, h;
	oc_color color;
	oc_image image;
	i32 sprite_index;
} Entity;

typedef struct {
	f64 timer;
	State next_state;
} Countdown;

typedef struct {
	i32 game_num;
	oc_str8 string;
	oc_color color;
	bool valid;
	oc_vec2 pos, target_pos;
	ColorCounts maxes;
} GameResult;

struct {
	bool operands_reached_targets;
	f32 opacity;
	f32 font_size_mod;
	f32 max_font_size_mod;
	f32 fade_delay;
} add_scene;

static char *file_data;
static u64 file_size;
static oc_surface surface;
static oc_canvas canvas;
static oc_font font;
static f32 font_size = 16;
static oc_color bg_color = {0, 0, 0, 1};
static oc_vec2 view_size = {512, 512};
static char *stream;
static Token token;
static State state;
static Entity elf;
static Entity elf_mouth;
static f64 mouth_timer;
static f32 mouth_timer_decay;
static Entity bag;
static Entity cubes[64];
static i32 num_cubes;
static Countdown countdown;
static f64 dt, last_timestamp;
static i32 current_game;
static GameResult game_results[100];
static GameResult result_sum;
static i32 num_game_results;
static oc_arena *string_arena;
static i32 cube_rate;
static oc_vec2 operators[100];
static i32 num_operators;

static oc_color colors[COLOR_COUNT] = {
	[COLOR_RED]    = { 1, 0, 0, 1},
	[COLOR_GREEN]  = { 0, 1, 0, 1},
	[COLOR_BLUE]   = { 0, 0, 1, 1},
	[COLOR_WHITE]  = { 1, 1, 1, 1},
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

static void add_cube_with_random_target(Color color) {
	assert(num_cubes < ARRAY_COUNT(cubes));
	Entity *cube = &cubes[num_cubes++];
	cube->color = colors[color];
	cube->target_pos.x = rand_range_u32(bag.pos.x, bag.pos.x + bag.w);
	cube->target_pos.y = rand_range_u32(50, bag.pos.y - 50);
}

static void prepare_game_results_for_addition(void) {
	i32 valid_count = 0;
	i32 sum = 0;
	i32 max_cols = 10;
	i32 col_width = 50;
	i32 row_height = font_size + 2;

	for (i32 i=0; i<ARRAY_COUNT(game_results); ++i) {
		GameResult *gr = &game_results[i];
		if (!gr->valid) continue;

		sum += gr->game_num;
		gr->target_pos.x = 5 + (valid_count % max_cols) * col_width;
		gr->target_pos.y = 5 + font_size + ((valid_count / max_cols) * row_height);
		++valid_count;

		operators[num_operators++] = (oc_vec2){
			.x = gr->target_pos.x + col_width/2 + 3,
			.y = gr->target_pos.y
		};
	}

	result_sum.string = oc_str8_pushf(string_arena, "%d", sum),
	result_sum.color = colors[COLOR_WHITE];
	result_sum.pos.x = 5 + (valid_count % max_cols) * col_width;
	result_sum.pos.y = 5 + font_size + ((valid_count / max_cols) * row_height);

	oc_text_metrics metrics = oc_font_text_metrics(
		font, font_size * add_scene.max_font_size_mod, result_sum.string);
	result_sum.target_pos.x = 0.5f * (view_size.x - metrics.ink.w);
	result_sum.target_pos.y = 0.5f * (view_size.y + metrics.ink.h);
}

static bool calculate_next_pull(void) {
	if (is_token(TOKEN_NEWLINE)) {
		next_token();
	}

	if (is_token(TOKEN_EOF)) {
		GameResult *gr = &game_results[current_game-1];
		if (gr->maxes.r <= 12 && gr->maxes.g <= 13 && gr->maxes.b <= 14) {
			gr->valid = true;
			gr->color = colors[COLOR_GREEN];
		} else {
			gr->valid = false;
			gr->color = colors[COLOR_RED];
		}
		prepare_game_results_for_addition();

		state = STATE_COUNTDOWN;
		countdown.timer = 1.5;
		countdown.next_state = STATE_ADDITION;
		return false;
	}

	if (is_token(TOKEN_GAME)) {
		if (current_game > 0) {
			GameResult *gr = &game_results[current_game-1];
			if (gr->maxes.r <= 12 && gr->maxes.g <= 13 && gr->maxes.b <= 14) {
				gr->color = colors[COLOR_GREEN];
				gr->valid = true;
			} else {
				gr->valid = false;
				gr->color = colors[COLOR_RED];
			}

			// speed up
			f32 max_cube_rate = 150;
			if (cube_rate < max_cube_rate) {
				cube_rate = (f32)cube_rate * 1.3;
				if (cube_rate > max_cube_rate) {
					cube_rate = max_cube_rate;
				}
				// mouth_timer_decay *= 0.9;
			}
		}

		current_game = parse_game_number();

		i32 max_rows = 28;
		i32 col = (current_game-1) / max_rows;
		i32 col_width = 2 * font_size - 8;
		i32 row = (current_game-1) % max_rows;
		i32 row_height = font_size + 2;

		game_results[num_game_results++] = (GameResult) {
			.game_num = current_game,
			.string = oc_str8_pushf(string_arena, "%d", current_game),
			.color = colors[COLOR_WHITE],
			.pos = { 5 + col * col_width, 3 + font_size + row * row_height }
		};
	}

	if (is_token(TOKEN_SEMICOLON)) {
		next_token();
	} 

	num_cubes = 0;
	ColorCounts reveal = parse_reveal();
	GameResult *gr = &game_results[current_game-1];
	if (reveal.r > gr->maxes.r) gr->maxes.r = reveal.r;
	if (reveal.g > gr->maxes.g) gr->maxes.g = reveal.g;
	if (reveal.b > gr->maxes.b) gr->maxes.b = reveal.b;

	for (i32 i=0; i<reveal.r; ++i) {
		add_cube_with_random_target(COLOR_RED);
	}
	for (i32 i=0; i<reveal.g; ++i) {
		add_cube_with_random_target(COLOR_GREEN);
	}
	for (i32 i=0; i<reveal.b; ++i) {
		add_cube_with_random_target(COLOR_BLUE);
	}
	return true;
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
	step_cubes(cube_rate);
	
	if (all_cubes_at_target()) {
		for (i32 i=0; i<num_cubes; ++i) {
			cubes[i].target_pos.x = bag.pos.x + bag.w/2;
			cubes[i].target_pos.y = bag.pos.y + 25;
		}
		state = STATE_RETURN_CUBES;
		// state = STATE_COUNTDOWN;
		// countdown.timer = 0;
		// countdown.next_state = STATE_RETURN_CUBES;
	}
}

static void update_return_cubes(void) {
	step_cubes(cube_rate);

	// check for state transition
	if (all_cubes_at_target()) {
		if (calculate_next_pull()) {
			state = STATE_PULL_CUBES;
		}
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

static void update_addition(void) {
	// fade in operators and sum / fade out invalid game nums
	if (add_scene.fade_delay > 0) {
		add_scene.fade_delay -= dt;
	} else {
		if (add_scene.opacity < 1) {
			f32 fade_rate = 0.3;
			add_scene.opacity += fade_rate * dt;
		} 
	}

	if (add_scene.operands_reached_targets) {
		// grow sum and move towards screen center
		GameResult *rs = &result_sum;
		if (add_scene.opacity >= 1) {
			if (rs->pos.x != rs->target_pos.x || rs->pos.y != rs->target_pos.y) {
				if (add_scene.font_size_mod < add_scene.max_font_size_mod) {
					f32 grow_rate = 0.8;
					add_scene.font_size_mod += grow_rate * dt;
				}
				if (vec2_dist(rs->pos, rs->target_pos) > 2) {
					f32 move_rate = 0.8;
					f32 dx = rs->target_pos.x - rs->pos.x;
					f32 dy = rs->target_pos.y - rs->pos.y;
					if (dx*dx < 1) dx /= dx; // ensure minimum dx of 1 
					if (dy*dy < 1) dy /= dy; // ensure minimum dy of 1
					rs->pos.x += dx * move_rate * dt;
					rs->pos.y += dy * move_rate * dt;
				} else {
					state = STATE_DONE;
					countdown.timer = 2;
				}
			}
		} 
		
	} else {
		add_scene.operands_reached_targets = true;
		for (i32 i=0; i<ARRAY_COUNT(game_results); ++i) {
			GameResult *gr = &game_results[i];
			if (gr->valid) {
				// move towards target
				if (gr->pos.x != gr->target_pos.x || gr->pos.y != gr->target_pos.y) {
					add_scene.operands_reached_targets = false;
					if (vec2_dist(gr->pos, gr->target_pos) < 1) {
						gr->pos = gr->target_pos;
					} else {
						f32 rate = 1.5;
						gr->pos.x += (gr->target_pos.x - gr->pos.x) * rate * dt;
						gr->pos.y += (gr->target_pos.y - gr->pos.y) * rate * dt;
					}
				}
			}
		}
	}
}

static void reset(void);

static void update_done(void) {
	if (countdown.timer > 0) {
		countdown.timer -= dt;
	} else {
		reset();
	}
}

static void draw_elf(void) {
	oc_rect elf_dest = { elf.pos.x, elf.pos.y, elf.w, elf.h };
	oc_image_draw(elf.image, elf_dest);

	// draw mouth
	oc_rect mouth_src = { elf_mouth.sprite_index * elf_mouth.w, 0, elf_mouth.w, elf_mouth.h };
	oc_rect mouth_dest = { elf_mouth.pos.x, elf_mouth.pos.y, elf_mouth.w, elf_mouth.h };
	oc_image_draw_region(elf_mouth.image, mouth_src, mouth_dest);
}

static void update_elf(void) {
	// slowly slide down the bag
	f32 slide_speed = 2;
	if (elf.pos.y + elf.h - 10 < bag.pos.y + bag.h) {
		elf.pos.y += slide_speed * dt;
		elf_mouth.pos.y += slide_speed * dt;
	}

	if (mouth_timer > 0) {
		mouth_timer -= dt;
	} else {
		mouth_timer = (rand_f32() * 3 + 0.5); // * mouth_timer_decay;
		elf_mouth.sprite_index = rand_range_u32(0, 3);
	}
}

static void draw_game_results(void) {
	oc_set_font_size(font_size);

	// draw game result nums
	for (i32 i=0; i<num_game_results; ++i) {
		GameResult *gr = &game_results[i];
		if (gr->valid || (state != STATE_ADDITION && state != STATE_DONE)) {
			oc_set_color(gr->color);
			oc_text_fill(gr->pos.x, gr->pos.y, gr->string);
		} 
	}

	// draw operators and sum
	if (state == STATE_ADDITION || state == STATE_DONE) {
		oc_set_color_rgba(1, 1, 1, add_scene.opacity);
		for (i32 i=0; i<num_operators-1; ++i) {
			oc_vec2 pos = operators[i];
			oc_text_fill(pos.x, pos.y, OC_STR8("+"));
		}

		oc_vec2 equals = operators[num_operators-1];
		oc_text_fill(equals.x, equals.y, OC_STR8("="));

		oc_set_color_rgba(0, 1, 0, add_scene.opacity);
		oc_set_font_size(font_size * add_scene.font_size_mod);
		oc_text_fill(result_sum.pos.x, result_sum.pos.y, result_sum.string);
	}
}


static void draw(void) {
    oc_canvas_select(canvas);
	oc_surface_select(surface);
	oc_set_color(bg_color);
	oc_clear();

	// draw cubes	
	for (i32 i=0; i<num_cubes; ++i) {
		oc_set_color(cubes[i].color);
		oc_rectangle_fill(cubes[i].pos.x, cubes[i].pos.y, cubes[i].w, cubes[i].h);
	}
	
	// draw bag
	oc_set_color(bag.color);
	oc_rectangle_fill(bag.pos.x, bag.pos.y, bag.w, bag.h);

	draw_elf();

	draw_game_results();

    oc_render(canvas);
    oc_surface_present(surface);
}

static void reset(void) {
	elf.pos.y = bag.pos.y - 120;

	elf_mouth.pos.y = elf.pos.y + 112;
	mouth_timer = 2.0;
	mouth_timer_decay = 1.0;

	init_stream(file_data);

	add_scene.operands_reached_targets = false;
	add_scene.opacity = 0;
	add_scene.font_size_mod = 1;
	add_scene.fade_delay = 2;

	cube_rate = 10;

	current_game = 0;

	num_cubes = 0;
	num_game_results = 0;
	num_operators = 0;

	calculate_next_pull();
	state = STATE_COUNTDOWN;
	countdown.timer = 0.5;
	countdown.next_state = STATE_PULL_CUBES;
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

	elf.image = oc_image_create_from_path(surface, OC_STR8("elf.png"), false);
	oc_vec2 elf_size = oc_image_size(elf.image);
	elf.w = elf_size.x;
	elf.h = elf_size.y;
	elf.pos.x = view_size.x - elf.w;
	elf.pos.y = bag.pos.y - 120;

	elf_mouth.image = oc_image_create_from_path(surface, OC_STR8("elf_mouths.png"), false);
	elf_mouth.w = 32;
	elf_mouth.h = 16;
	elf_mouth.pos.x = elf.pos.x + 72;
	elf_mouth.pos.y = elf.pos.y + 112;
	mouth_timer = 2.0;
	mouth_timer_decay = 1.0;

	for (i32 i=0; i<ARRAY_COUNT(cubes); ++i) {
		Entity *cube = &cubes[i];
		cube->pos.x = bag.pos.x + bag.w/2;
		cube->pos.y = bag.pos.y + 20;
		cube->w = 20;
		cube->h = 20;
	}

	char *filename = "input.txt";
	if (!read_entire_file(filename, &file_data, &file_size)) {
		oc_log_error("failed to read file %s\n", filename);
	}

	init_stream(file_data);

	// part_one(file_data, file_size);
	// part_two(file_data, file_size);

    last_timestamp = oc_clock_time(OC_CLOCK_MONOTONIC);

	add_scene.font_size_mod = 1;
	add_scene.max_font_size_mod = 5;

	reset();

	// for (i32 i=0; i<ARRAY_COUNT(game_results); ++i) {
		// i32 max_rows = 28;
		// i32 col = i / max_rows;
		// i32 col_width = 2 * font_size - 8;
		// i32 row = i % max_rows;
		// i32 row_height = font_size + 2;

		// game_results[i] = (GameResult){
			// .game_num = i,
			// .string = oc_str8_pushf(string_arena, "%d", i),
			// .color = colors[COLOR_GREEN],
			// .valid = (i % 2) == 0,
			// .pos = { 5 + col * col_width, 3 + font_size + row * row_height }
		// };
	// }
	// prepare_game_results_for_addition();
	// num_game_results = ARRAY_COUNT(game_results);
	// state = STATE_ADDITION;

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
		case STATE_ADDITION:     update_addition();     break;
		case STATE_DONE:         update_done();         break;
		default:                 assert(0);             break;
	}

	update_elf();
	draw();
}



