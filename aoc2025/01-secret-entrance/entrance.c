#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE 1024
#define SCREEN_MID  (SCREEN_SIZE/2)
#define PI  3.14159265358

#define SILVER 0xB8B8B8
#define GREEN  0x00B200
#define RED    0x0000BB

void part_one(U8 *input) {
	int result = 0;
	int dial = 50;
	char dir = 0;
	int val = 0;
	int n = 0;
	while (sscanf(input, "%c%d%n", &dir, &val, &n) != EOF) {
		if (dir == 'L') {
			dial -= val;
			while (dial < 0) dial += 100;
		} else {
			dial += val;
			while (dial > 99) dial -= 100;
		}
		if (dial == 0) ++result;
		input += n;
		while(isspace(*input)) ++input;
	}
	printf("part one: %d\n", result);
}

void part_two(U8 *input) {
	int result = 0;
	int dial = 50;
	char dir = 0;
	int val = 0;
	int n = 0;
	while (sscanf(input, "%c%d%n", &dir, &val, &n) != EOF) {
		while (val > 0) {
			if (dir == 'L') {
				--dial;
				if (dial == -1) dial = 99;
				if (dial == 0) ++result;
			} else {
				++dial;
				if (dial == 100) dial = 0;
				if (dial == 0) ++result;
			}
			--val;
		}
		input += n;
		while(isspace(*input)) ++input;
	}
	printf("part two: %d\n", result);
}

typedef struct {
	F64 x0, x1, y0, y1;
	U32 color;
} Line;

Line create_tick_mark(F64 angle, F64 radius, U32 color) {
	Line line = {0};
	F64 offset = 15;
	F64 tick_length = 45;
	F64 length0 = radius - offset - tick_length;
	F64 length1 = length0 + tick_length;

	line.x0 = SCREEN_MID + length0 * cos(angle);
	line.y0 = SCREEN_MID - length0 * sin(angle);
	line.x1 = SCREEN_MID + length1 * cos(angle);
	line.y1 = SCREEN_MID - length1 * sin(angle);
	line.color = color;

	return line;
}

Line create_nub(F64 angle, F64 radius, U32 color) {
	Line line = {0};
	F64 length = 8;
	F64 length0 = radius-1;
	F64 length1 = radius+length;

	line.x0 = SCREEN_MID + length0 * cos(angle);
	line.y0 = SCREEN_MID - length0 * sin(angle);
	line.x1 = SCREEN_MID + length1 * cos(angle);
	line.y1 = SCREEN_MID - length1 * sin(angle);
	line.color = color;

	return line;
}

void draw_dial(F64 angle) {
	F64 r = 420;

	// draw dial border
	F64 r_border = r + 50;
	drawcircle(SCREEN_MID, SCREEN_MID, r_border, SILVER);

	// draw indicator
	drawtri(
		SCREEN_MID, SCREEN_MID - r_border + 35,
		SCREEN_MID-12, SCREEN_MID - r_border + 15,
		SCREEN_MID+12, SCREEN_MID - r_border + 15,
		GREEN);

	// draw outer dial
	drawcircle(SCREEN_MID, SCREEN_MID, r, RED);

	// draw inner dial
	U32 color_inner = 0x000033;
	F64 r_inner = 128;
	drawcircle(SCREEN_MID, SCREEN_MID, r_inner, color_inner);
	int nubs = 14;
	for (int i=0; i<nubs; ++i) {
		F64 nub_angle = (2 * PI * i / nubs) + PI/2 + angle;
		U32 color = i == 0 ? GREEN : 0xFFFFFF;
		Line line = create_nub(nub_angle, r_inner, color_inner);
		drawline(line.x0, line.y0, line.x1, line.y1, 30, line.color);
	}

	// draw tick marks
	int interval = 5;
	for (int i=0; i<100; i += interval) {
		F64 tick_angle = (2 * PI * i / 100) + PI/2 + angle;
		F64 tick_width = i == 0 ? 15 : 5;
		U32 color = i == 0 ? GREEN : 0xFFFFFF;
		Line line = create_tick_mark(tick_angle, r, color);
		drawline(line.x0, line.y0, line.x1, line.y1, tick_width, line.color);
	}
}

void visualize(U8 *input) {
	setupgif(1, 1, "entrance.gif");

	int result = 0;
	int dial = 50;
	char dir = 0;
	int val = 0;
	int n = 0;
	U64 skip_frames = 1;
	U64 max_skip_frames = 2;
	U64 frames = 0;
	F64 angle = 0;
	while (sscanf(input, "%c%d%n", &dir, &val, &n) != EOF) {
		int start_val = val;
		while (val > 0) {
			if (dir == 'L') {
				--dial;
				if (dial == -1) dial = 99;
				if (dial == 0) ++result;
			} else {
				++dial;
				if (dial == 100) dial = 0;
				if (dial == 0) ++result;
			}
			--val;

			// draw
			angle = 2 * PI * dial / 100;
			clear();
			draw_dial(angle);
			drawstringf(SCREEN_MID-35, SCREEN_MID-30, 5, 0xFFFFFF, "%2d", dial);
			drawstringf(25, 25, 5, 0xFFFFFF, "%2d", result);
			drawstringf(825, 35, 5, SILVER, "%c%d", dir, start_val);

			if (val > 10) {
				F64 t = (F64)val / (F64)start_val;
				skip_frames = 1 + (U64)(6.0*t*(1.0-t) * max_skip_frames); // first derivative of cubic smoothstep
			} else {
				skip_frames = 1;
			}

			if ((frames++ % skip_frames) == 0)
				nextframe();
		}

		// pause on number
		nextframe();
		nextframe();

		input += n;
		while(isspace(*input)) ++input;
	}

	// end frame
	clear();
	draw_dial(angle);
	drawstringf(SCREEN_MID-35, SCREEN_MID-30, 5, 0xFFFFFF, "%2d", dial);
	drawstringf(25, 25, 5, 0xFFFFFF, "%2d", result);
	strokerect(45, 15, 115, 90, 2, 0x00FF00);
	for (int i=0; i<16; ++i) nextframe();


	endgif();
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

	char *file_name = argv[1];
	
	char *file_data;
	U64 file_size;
	if (!read_entire_file(file_name, &file_data, &file_size)) {
		fatal("failed to read file %s", file_name);
	}

	part_one(file_data);
	part_two(file_data);
	visualize(file_data);

    return 0;
}
