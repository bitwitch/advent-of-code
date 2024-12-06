#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define GREEN     0xFF008200
#define RED       0xFF2A2ACD
#define BLUE      0xFFC70303
#define YELLOW    0xFF12c29F

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024
#define FONT_SIZE    12
#define PAD_LEFT     100
#define PAD_TOP      80
#define CHAR_WIDTH   (8 * FONT_SIZE)
#define CHAR_HEIGHT  (12 * FONT_SIZE)

static void part_one(BUF(int **reports)) {
	int safe_count = 0;

	for (int j=0; j<buf_len(reports); ++j) {
		BUF(int *report) = reports[j];

		bool is_increasing = report[1] > report[0];
		bool is_safe = true;
		for (int i=0; i<buf_len(report) - 1; ++i) {
			int curr = report[i];
			int next = report[i+1];

			bool is_sequence = is_increasing ? (next > curr) : (next < curr);
			int diff = abs(curr - next);
			bool is_acceptable_diff = diff >= 1 && diff <= 3;

			if (!is_sequence || !is_acceptable_diff) {
				is_safe = false;
				break;
			}
		}

		if (is_safe) {
			safe_count += 1;
		}
	}

	printf("part one: %d reports are safe.\n", safe_count);
}

static bool report_is_safe(BUF(int *report)) {
	bool is_safe = true;
	bool is_increasing = report[1] > report[0];
	for (int i=0; i<buf_len(report) - 1; ++i) {
		int curr = report[i];
		int next = report[i+1];

		bool is_sequence = is_increasing ? (next > curr) : (next < curr);
		int diff = abs(curr - next);
		bool is_acceptable_diff = diff >= 1 && diff <= 3;

		if (!is_sequence || !is_acceptable_diff) {
			is_safe = false;
			break;
		}
	}
	return is_safe;
}

static bool report_is_safe_remove_index(BUF(int *report), int index) {
	bool is_safe = true;

	int i1, i2;
	if (index == 0) { 
		i1 = 1; 
		i2 = 2;
	} else if (index == 1) {
		i1 = 0; 
		i2 = 2;
	} else {
		i1 = 0; 
		i2 = 1;
	}
	bool is_increasing = report[i2] > report[i1];

	for (int i=0; i<buf_len(report) - 1; ++i) {
		if (i == index) continue;
		int curr = report[i];
		
		int next = report[i+1];
		if (i+1 == index) {
			if (i+2 >= buf_len(report)) {
				break;
			}
			next = report[i+2];
		}

		bool is_sequence = is_increasing ? (next > curr) : (next < curr);
		int diff = abs(curr - next);
		bool is_acceptable_diff = diff >= 1 && diff <= 3;

		if (!is_sequence || !is_acceptable_diff) {
			is_safe = false;
			break;
		}
	}
	return is_safe;
}

static void part_two(BUF(int **reports)) {
	int safe_count = 0;

	for (int j=0; j<buf_len(reports); ++j) {
		BUF(int *report) = reports[j];

		if (report_is_safe(report)) {
			safe_count += 1;
		} else {
			for (int i=0; i<buf_len(report); ++i) {
				if (report_is_safe_remove_index(report, i)) {
					safe_count += 1;
					break;
				}
			}
		}
	}

	printf("part two: %d reports are safe.\n", safe_count);
}

typedef enum {
	REPORT_MODE_DEFAULT,
	REPORT_MODE_SAFE,
	REPORT_MODE_SAFE_WITH_DAMPENER,
} ReportMode;

typedef struct {
	BUF(int *levels);
	bool is_safe;
	bool is_safe_with_dampener;
	bool highlight_dampener_index;
	int dampener_index;
	U32 bg_color;
} Report;

static void draw_row_cursor(int row) {
	int y = PAD_TOP + row*CHAR_HEIGHT + CHAR_HEIGHT/2;
	drawarrow(800, y, 700, y, 10, BLUE);
}

static void draw_level(int level, U32 color, int row, int col) {
	int x = PAD_LEFT + col * CHAR_WIDTH;
	int y = PAD_TOP + row * CHAR_HEIGHT;
	drawstringf(x, y, FONT_SIZE, color, "%d", level);
}


static void draw_reports(BUF(Report *reports)) {
	for (int j=0; j<buf_len(reports); ++j) {
		Report report = reports[j];
		int report_y = PAD_TOP + j * CHAR_HEIGHT;

		drawbox(PAD_LEFT, report_y, CHAR_WIDTH * buf_len(report.levels), CHAR_HEIGHT, report.bg_color);

		for (int i=0; i<buf_len(report.levels); ++i) {
			int x = PAD_LEFT + i * CHAR_WIDTH;
			bool is_dampened_index = report.highlight_dampener_index && report.dampener_index == i;
			U32 text_color = is_dampened_index ? 0xFF000000: 0xFFFFFFFF;
			drawstringf(x, report_y, FONT_SIZE, text_color, "%d", report.levels[i]);
		}
	}
}

static void draw_background(void) {
	drawbox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xFFBBBBBB);

	int screen_x = PAD_LEFT - 25;
	int screen_y = PAD_TOP - 15;
	drawbox(screen_x, screen_y, 
			SCREEN_WIDTH - 2 * screen_x, SCREEN_HEIGHT - 2 * screen_y, 
			0xFF000000);

	int offx = 40;
	int offy = 40;
	U32 bolt_r = 16;
	U32 bolt_color = 0xFF696969;
	drawcircle(offx, offy, bolt_r, bolt_color);
	drawcircle(offx, SCREEN_HEIGHT - offy, bolt_r, bolt_color);
	drawcircle(SCREEN_WIDTH - offx, offy, bolt_r, bolt_color);
	drawcircle(SCREEN_WIDTH - offx, SCREEN_HEIGHT - offy, bolt_r, bolt_color);
}

static void visualize_small(BUF(int **raw_reports)) {
	BUF(Report *reports) = NULL;
	for (int j=0; j<buf_len(raw_reports); ++j) {
		Report report = {0};
		report.levels = raw_reports[j];
		report.is_safe = report_is_safe(report.levels);
		if (!report.is_safe) {
			for (int i=0; i<buf_len(report.levels); ++i) {
				if (report_is_safe_remove_index(report.levels, i)) {
					report.is_safe_with_dampener = true;
					report.dampener_index = i;
					break;
				}
			}
		}
		buf_push(reports, report);
	}

	S64 frames = 0;

	// draw stationary
	draw_background();
	draw_reports(reports);
	for(frames = 0; frames < 15; ++frames) {
		nextframe();
	}

	int check_report_delay = 10;
	int check_report_counter = check_report_delay;

	// do safe check
	for (int row=0; row<buf_len(reports);) {
		clear();
		draw_background();
		draw_reports(reports);
		draw_row_cursor(row);
		nextframe();

		if (--check_report_counter <= 0) {
			Report *report = &reports[row];
			report->bg_color = report->is_safe ? GREEN : RED;
			check_report_counter = check_report_delay;
			++row;
		} 
	}

	int check_level_delay = 2;
	int check_level_counter = check_level_delay;

	// do safe check with dampener
	int col = 0;
	for (int row=0; row<buf_len(reports);) {
		Report *report = &reports[row];

		clear();
		draw_background();
		draw_reports(reports);
		draw_row_cursor(row);
		if (!report->is_safe && col >= 0 && col < buf_len(report->levels)) {
			draw_level(report->levels[col], BLUE, row, col);
		}
		nextframe();

		if (--check_level_counter <= 0) {
			if (!report->is_safe && report_is_safe_remove_index(report->levels, col)) {
				report->bg_color = YELLOW;
				report->highlight_dampener_index = true;
			}

			check_level_counter = check_level_delay;
			++col;
			if (col > buf_len(reports[row].levels)) {
				++row;
				col = 0;
			}
		} 
	}

	// draw stationary
	clear();
	draw_background();
	draw_reports(reports);
	for(frames = 0; frames < 15; ++frames) {
		nextframe();
	}
}


static void draw_report_lines(BUF(Report *reports), int max_row, F64 scroll_y) {
	F64 line_width = 1;
	for (int j=0; j<buf_len(reports) && j <= max_row; ++j) {
		Report report = reports[j];
		F64 y = j * line_width - scroll_y;
		if (y >= -line_width && y < SCREEN_WIDTH) {
			drawline(0, y, SCREEN_WIDTH, y, line_width, report.bg_color);
		}
	}
}

static void shuffle_reports(BUF(Report *reports)) {
	for (int i = buf_len(reports) - 1; i > 0; --i) {
		int j = (int)rand_range_u32(0, i);
		Report tmp = reports[i];
		reports[i] = reports[j];
		reports[j] = tmp;
	}
}

static void visualize_large(BUF(int **raw_reports)) {
	BUF(Report *reports) = NULL;
	for (int j=0; j<buf_len(raw_reports); ++j) {
		Report report = {0};
		report.levels = raw_reports[j];
		report.is_safe = report_is_safe(report.levels);

		if (report.is_safe) {
			report.bg_color = GREEN;
		} else {
			report.bg_color = RED;
			for (int i=0; i<buf_len(report.levels); ++i) {
				if (report_is_safe_remove_index(report.levels, i)) {
					report.is_safe_with_dampener = true;
					report.dampener_index = i;
					break;
				}
			}
		}
		buf_push(reports, report);
	}

	shuffle_reports(reports);

	int delay = 1;
	int counter = delay;

	for (int row=0; row<buf_len(reports);) {
		draw_report_lines(reports, row, 0);
		nextframe();

		if (--counter <= 0) {
			row += 10;
			counter = delay;
		}
	}
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

	BUF(int **reports) = NULL;

	while (!is_token(TOKEN_EOF)) {
		BUF(int *report) = NULL;
		do {
			int level = (int)parse_int();
			buf_push(report, level);
		} while (!match_token('\n'));
		buf_push(reports, report);
	}


	part_one(reports);
	part_two(reports);

    setupgif(0, 1, "reports.gif");
	visualize_small(reports);
	// visualize_large(reports);
	endgif();

	return 0;
}
