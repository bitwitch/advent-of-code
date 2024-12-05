#include "..\common.c"
#include "..\lex.c"
#include "..\..\base\base_inc.h"
#include "..\..\base\base_inc.c"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

typedef struct {
	F32 x, y, target_x, target_y;
	S64 num;
} Cell;

typedef struct {
	F32 center_x, center_y;
	int w, h;
	int target_w;
} Bar;

typedef struct {
	S64 num;
	int start_index;
} NumIndex;

static int compare_num_index(const void* a, const void* b) {
    NumIndex arg1 = *(const NumIndex*)a;
    NumIndex arg2 = *(const NumIndex*)b;
    if (arg1.num < arg2.num) return -1;
    if (arg1.num > arg2.num) return 1;
    return 0;
}

static int compare_s64(const void* a, const void* b) {
    S64 arg1 = *(const S64*)a;
    S64 arg2 = *(const S64*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

static int lerp(int a, int b, float t) {
	return a + (int)(t * (b - a));
}
static F32 lerpf(F32 a, F32 b, F32 t) {
	return a + t * (b - a);
}


static void part_one(S64 *nums1, S64 *nums2) {
	U64 sum = 0;
	
	assert(buf_len(nums1) == buf_len(nums2));
	for (S64 i=0; i<buf_len(nums1); ++i) {
		S64 diff = llabs(nums1[i] - nums2[i]);
		sum += diff;
	}

	printf("part one: %llu\n", sum);
}

static void part_two(S64 *nums1, S64 *nums2) {
	Map frequency = {0};

	for (S64 i=0; i<buf_len(nums2); ++i) {
		S64 n = nums2[i];
		U64 count = (U64)map_get(&frequency, (void*)n);
		map_put(&frequency, (void*)n, (void*)(count + 1));
	}

	U64 similarity = 0;
	for (S64 i=0; i<buf_len(nums1); ++i) {
		S64 n = nums1[i];
		U64 count = (U64)map_get(&frequency, (void*)n);
		similarity += n * count;
	}

	printf("part two: %llu\n", similarity);
}

static void draw_cells(BUF(Cell *cells), int font_size, U32 color, float scroll_y) {
	int cell_width = font_size * 8;
	int cell_height = font_size * 12;
	for (S64 i=0; i<buf_len(cells); ++i) {
		Cell cell = cells[i];
		int y = (int)(cell.y - scroll_y);

		if (cell.x < SCREEN_WIDTH && y < SCREEN_HEIGHT && y > -cell_height) {
			drawstringf((int)cell.x, y, font_size, color, "%lld", cell.num);  
		}
	}
}

static void draw_bars(BUF(Bar *bars), U32 color, float scroll_y) {
	for (S64 i=0; i<buf_len(bars); ++i) {
		Bar bar = bars[i];
		int x = (int)(bar.center_x - bar.w/2.0f);
		int y = (int)(bar.center_y - bar.h/2.0f - scroll_y);

		if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT && y > -bar.h) {
			drawbox(x, y, bar.w, bar.h, 0xFF00FF00);
		}
	}
}

static void grow_bars(BUF(Bar *bars), float t) {
	for (S64 i=0; i<buf_len(bars); ++i) {
		Bar bar = bars[i];
		bars[i].w = lerp(bar.w, bar.target_w, t);
	}
}

static void move_cells(BUF(Cell *cells), float t) {
	for (S64 i=0; i<buf_len(cells); ++i) {
		Cell cell = cells[i];
		cells[i].x = lerpf(cell.x, cell.target_x, t);
		cells[i].y = lerpf(cell.y, cell.target_y, t);
	}
}

static void visualize_small(BUF(S64 *nums1), BUF(S64 *nums2), int font_size, int pad_left, int pad_top, int max_bar_width) {
    setupgif(0, 1, "historian_small.gif");

	// generate "cells"
	int cell_width = font_size * 8;
	int cell_height = font_size * 12;
	BUF(NumIndex *list1) = NULL;
	BUF(NumIndex *list2) = NULL;
	for (int i=0; i<buf_len(nums1); ++i) {
		buf_push(list1, (NumIndex){ nums1[i], i });
	}
	for (int i=0; i<buf_len(nums2); ++i) {
		buf_push(list2, (NumIndex){ nums2[i], i });
	}
	qsort(list1, buf_len(list1), sizeof(NumIndex), compare_num_index);
	qsort(list2, buf_len(list2), sizeof(NumIndex), compare_num_index);

	BUF(Cell *cells) = NULL;
	for (int i=0; i<buf_len(list1); ++i) {
		NumIndex item = list1[i];
		buf_push(cells, (Cell){
			.x = (F32)pad_left,
			.y = (F32)item.start_index * cell_height + pad_top,
			.target_x = (F32)pad_left,
			.target_y = (F32)i * cell_height + pad_top,
			.num = item.num,
		});
	}
	for (int i=0; i<buf_len(list2); ++i) {
		NumIndex item = list2[i];
		buf_push(cells, (Cell){
			.y = (F32)item.start_index * cell_height + pad_top,
			.x = pad_left + 9.0f * cell_width,
			.target_x = pad_left + 9.0f * cell_width,
			.target_y = (F32)i * cell_height + pad_top,
			.num = item.num,
		});
	}

	// generate "bars"
	int bar_unit_width = cell_width;
	int bar_height = cell_height - 2;
	int longest = 0;
	BUF(Bar *bars) = NULL;
	for (int i=0; i<buf_len(list1); ++i) {
		NumIndex n1 = list1[i];
		NumIndex n2 = list2[i];
		Bar bar = {0};
		bar.center_x = SCREEN_WIDTH / 2;
		bar.center_y = pad_top + (i*cell_height) + (bar_height/2.0f);
		bar.w = 0;
		bar.h = bar_height;
		bar.target_w = abs((int)(n2.num - n1.num)) * bar_unit_width;
		if (bar.target_w > longest) longest = bar.target_w;
		buf_push(bars, bar);
	}

	// scale bars to screen
	float bar_scale_ratio = (float)max_bar_width / (float)longest;
	for (int i=0; i<buf_len(bars); ++i) {
		bars[i].target_w = (int)(bars[i].target_w * bar_scale_ratio);
	}

	S64 frames = 0;
	U32 cell_color = 0xFF0000FF;

	// draw unsorted lists
	for (frames = 0; frames < 50; ++frames) {
		draw_cells(cells, font_size, cell_color, 0);
		nextframe();
	}

	// draw transition to sorted lists
	for (frames = 0; frames < 69; ++frames) {
		clear();
		draw_cells(cells, font_size, cell_color, 0);
		move_cells(cells, 0.17f);
		nextframe();
	}

	// draw diff bars
	U32 bar_color = 0xFF00FF00;
	for (frames = 0; frames < 80; ++frames) {
		clear();

		draw_cells(cells, font_size, cell_color, 0);
		draw_bars(bars, bar_color, 0);

		move_cells(cells, 0.17f);
		grow_bars(bars, 0.19f);

		nextframe();
	}

	// black frames
	clear();
	for (frames = 0; frames < 20; ++frames) {
		nextframe();
	}

    endgif();
}

static void visualize_large(BUF(S64 *nums1), BUF(S64 *nums2), int font_size, int pad_left, int pad_top, int max_bar_width) {
    setupgif(0, 1, "historian_large.gif");

	// generate "cells"
	int cell_width = font_size * 8;
	int cell_height = font_size * 12;
	BUF(NumIndex *list1) = NULL;
	BUF(NumIndex *list2) = NULL;
	for (int i=0; i<buf_len(nums1); ++i) {
		buf_push(list1, (NumIndex){ nums1[i], i });
	}
	for (int i=0; i<buf_len(nums2); ++i) {
		buf_push(list2, (NumIndex){ nums2[i], i });
	}
	qsort(list1, buf_len(list1), sizeof(NumIndex), compare_num_index);
	qsort(list2, buf_len(list2), sizeof(NumIndex), compare_num_index);

	BUF(Cell *cells) = NULL;
	for (int i=0; i<buf_len(list1); ++i) {
		NumIndex item = list1[i];
		buf_push(cells, (Cell){
			.x = (F32)pad_left,
			.y = (F32)item.start_index * cell_height + pad_top,
			.target_x = (F32)pad_left,
			.target_y = (F32)i * cell_height + pad_top,
			.num = item.num,
		});
	}
	for (int i=0; i<buf_len(list2); ++i) {
		NumIndex item = list2[i];
		buf_push(cells, (Cell){
			.y = (F32)item.start_index * cell_height + pad_top,
			.x = pad_left + 900.0f,
			.target_x = pad_left + 900.0f,
			// .x = pad_left + 9 * cell_width,
			// .target_x = pad_left + 9 * cell_width,
			.target_y = (F32)i * cell_height + pad_top,
			.num = item.num,
		});
	}

	// generate "bars"
	int bar_unit_width = cell_width;
	int bar_height = cell_height - 2;
	int longest = 0;
	BUF(Bar *bars) = NULL;
	for (int i=0; i<buf_len(list1); ++i) {
		NumIndex n1 = list1[i];
		NumIndex n2 = list2[i];
		Bar bar = {0};
		bar.center_x = SCREEN_WIDTH / 2;
		bar.center_y = pad_top + (i*cell_height) + (bar_height/2.0f);
		bar.w = 0;
		bar.h = bar_height;
		bar.target_w = abs((int)(n2.num - n1.num)) * bar_unit_width;
		if (bar.target_w > longest) longest = bar.target_w;
		buf_push(bars, bar);
	}

	// scale bars to screen
	float bar_scale_ratio = (float)max_bar_width / (float)longest;
	for (int i=0; i<buf_len(bars); ++i) {
		bars[i].target_w = (int)(bars[i].target_w * bar_scale_ratio);
	}

	for (int i=0; i<buf_len(cells); ++i) {
		cells[i].x = cells[i].target_x;
		cells[i].y = cells[i].target_y;
	}
	// for (int i=0; i<buf_len(bars); ++i) {
		// bars[i].w = bars[i].target_w;
	// }

	S64 frames = 0;
	int stationary_frames = 25;
	int fade_frames = 25;
	U32 cell_color = 0x000000FF;
	U32 bar_color = 0x0000FF00;

	// draw stationary
	clear();
	draw_cells(cells, font_size, cell_color, 0);
	draw_bars(bars, bar_color, 0);
	for (frames = 0; frames < stationary_frames; ++frames) {
		nextframe();
	}

	// grow bars
	for (frames = 0; frames < 60; ++frames) {
		clear();
		draw_cells(cells, font_size, cell_color, 0);
		draw_bars(bars, bar_color, 0);
		grow_bars(bars, 0.069f);
		nextframe();
	}

	// scroll
	frames = 0;
	int skip_frames = 1;
	int list_height = cell_height * buf_len(nums1);
	float scroll_y = 0;
	for (scroll_y = 0; scroll_y < list_height - SCREEN_HEIGHT; scroll_y += 25.0f) {
		clear();
		draw_cells(cells, font_size, cell_color, scroll_y);
		draw_bars(bars, bar_color, scroll_y);

		if (frames > 800) {
			skip_frames = 5;
		} else if (frames > 400) {
			skip_frames = 4;
		} else if (frames > 200) {
			skip_frames = 3;
		} else if (frames > 100) {
			skip_frames = 2;
		} 
		if ((frames++ % skip_frames) == 0) {
			nextframe();
		}
	}
	printf("%lld frames in scroll\n", frames);

	// draw stationary
	for (frames = 0; frames < stationary_frames; ++frames) {
		// clear();
		// draw_cells(cells, font_size, scroll_y);
		// draw_bars(bars, scroll_y);
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

	init_lexer(file_name, file_data);

	BUF(S64 *nums1) = NULL;
	BUF(S64 *nums2) = NULL;

	while (!is_token(TOKEN_EOF)) {
		S64 n1 = (int)parse_int();
		S64 n2 = (int)parse_int();
		if (is_token('\n')) {
			next_token();
		}
		buf_push(nums1, n1);
		buf_push(nums2, n2);
	}

	BUF(S64 *nums1_sorted) = NULL;
	BUF(S64 *nums2_sorted) = NULL;
	for (S64 i=0; i<buf_len(nums1); ++i) buf_push(nums1_sorted, nums1[i]);
	for (S64 i=0; i<buf_len(nums2); ++i) buf_push(nums2_sorted, nums2[i]);

	qsort(nums1_sorted, buf_len(nums1_sorted), sizeof(S64), compare_s64);
	qsort(nums2_sorted, buf_len(nums2_sorted), sizeof(S64), compare_s64);

	part_one(nums1_sorted, nums2_sorted);
	part_two(nums1_sorted, nums2_sorted);

	// int font_size = 12;
	// int pad_left = 32;
	// int pad_top = 75;
	// int max_bar_width = 700;
	// visualize_small(nums1, nums2, font_size, pad_left, pad_top, max_bar_width);

	int font_size = 2;
	int pad_left = 25;
	int pad_top = 1;
	int max_bar_width = 700;
	visualize_large(nums1, nums2, font_size, pad_left, pad_top, max_bar_width);

	return 0;
}
