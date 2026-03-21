#include "..\common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"
/*
   - only nums with even number of digits can contain repeats
   - does its first n digits match its last n digits
*/

#define SCREEN_SIZE 1024
#define SCREEN_MID  (SCREEN_SIZE/2)
#define FONT_UNIT_X 8
#define FONT_UNIT_Y 12
#define MAX_ROWS    42
#define MAX_COLS    4

typedef struct {
	U64 min, max;
} Range;

int digits(U64 n) {
	int result = 0;
	while (n > 0) {
		n /= 10;
		++result;
	}
	return result;
}

U64 first_n_digits(U64 n, int count) {
	count = digits(n) - count;
	int divisor = 1;
	for (int i=0; i<count; ++i) divisor *= 10;
	return n / divisor;
}

U64 last_n_digits(U64 n, int count) {
	int divisor = 1;
	for (int i=0; i<count; ++i) divisor *= 10;
	return n % divisor;
}


void part_one(BUF(Range *ranges)) {
	U64 result = 0;

	for (int i=0; i<buf_len(ranges); ++i) {
		for (U64 n = ranges[i].min; n <= ranges[i].max; ++n) {
			int num_digits = digits(n);
			if ((num_digits % 2) == 0) {
				U64 left = first_n_digits(n, num_digits/2);
				U64 right = last_n_digits(n, num_digits/2);
				if (left == right) result += n;
			}
		}		
	}

	printf("part one: %llu\n", result);
}

void part_two(BUF(Range *ranges)) {
	U64 result = 0;
	for (int i=0; i<buf_len(ranges); ++i) {
		for (U64 n = ranges[i].min; n <= ranges[i].max; ++n) {
			int num_digits = digits(n);
			for (int j=1; j<num_digits; ++j) {
				if ((num_digits % j) == 0) {
					bool valid = false;
					U64 val = first_n_digits(n, j);
					U64 remaining = n;
					for(int k=0; k<num_digits/j; ++k) {
						U64 next_val = first_n_digits(remaining, j);
						if (val != next_val) {
							valid = true;
							break;
						}
						remaining = last_n_digits(remaining, digits(remaining) - j);
					}

					if (!valid) {
						result += n;
						break;
					}
				}
			}
		}		
	}

	printf("part two: %llu\n",  result);
}

void draw_num(U64 n, U32 color) {
	int num_digits = digits(n);
	int min_pad = 50;
	int pixels_per_digit = (SCREEN_SIZE - min_pad)/ num_digits;
	int font_size = pixels_per_digit / FONT_UNIT_X;
	int char_width = font_size * FONT_UNIT_X;
	int char_height = font_size * FONT_UNIT_Y;
	int x = 20 + (SCREEN_SIZE - (char_width * num_digits)) / 2;
	int y = SCREEN_MID - char_height/2;
	drawstringf(x, y, font_size, color, "%llu", n);
}

void draw_repeats(U64 *repeats, int num_repeats) {
	int font_size = 2;
	int col_width = SCREEN_SIZE / MAX_COLS;
	for (int i=0; i<num_repeats; ++i) {
		int row = i % MAX_ROWS;
		int y = row * font_size * (int)FONT_UNIT_Y;
		int col = i / MAX_ROWS;
		int x = (col * col_width);
		drawstringf(x, y, font_size, 0x00AD00, "%llu", repeats[i]);
	}
}

void draw_frame(U64 n, U64 *repeats, int num_repeats) {
	clear();
	draw_num(n, 0xFFFFFF);
	draw_repeats(repeats, num_repeats);
	nextframe();
}

void visualize(BUF(Range *ranges)) {
	setupgif(1, 1, "gift_shop.gif");

	U64 result = 0;
	U64 repeats[MAX_ROWS * MAX_COLS] = {0};
	int num_repeats = 0;
	int repeat_index = 0;
	int skip_frames = 1913;
	int frames = 0;
	bool drew = false;
	for (int i=0; i<buf_len(ranges); ++i) {
		for (U64 n = ranges[i].min; n <= ranges[i].max; ++n) {
			drew = false;
			int num_digits = digits(n);
			for (int j=1; j<num_digits; ++j) {
				if ((num_digits % j) == 0) {
					bool valid = false;
					U64 val = first_n_digits(n, j);
					U64 remaining = n;
					for(int k=0; k<num_digits/j; ++k) {
						U64 next_val = first_n_digits(remaining, j);
						if (val != next_val) {
							valid = true;
							break;
						}
						remaining = last_n_digits(remaining, digits(remaining) - j);
					}

					if (!valid) {
						result += n;
						repeats[repeat_index] = n;
						if (num_repeats < ARRAY_COUNT(repeats)) {
							++num_repeats;
						}
						if (++repeat_index >= ARRAY_COUNT(repeats)) repeat_index = 0;
						draw_frame(n, repeats, num_repeats);
						drew = true;
						break;
					}
				}
			}

			if (frames++ % skip_frames == 0 && !drew) {
				draw_frame(n, repeats, num_repeats);
			}
		}		
	}

	for (int i=0; i<32; ++i) nextframe();
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


	Range *ranges = NULL;
	for (;;) {
		char *str = chop_by_delimiter(&file_data, ",");
		eat_spaces(&str);
		if (*str == 0) break;

		Range r = {0};
		if (sscanf(str, "%llu-%llu", &r.min, &r.max) != 2) {
			fatal("failed to parse range: %s", str);
		}
		buf_push(ranges, r);
	}

	printf("found %d ranges\n", buf_len(ranges));

	part_one(ranges);
	part_two(ranges);
	visualize(ranges);

	return 0;
}
