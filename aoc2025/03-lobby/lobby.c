#include "..\common.c"
#include "..\..\base\base_inc.h"
#include "..\..\base\base_inc.c"

#define SCREEN_SIZE 1024
#define SCREEN_MID  (1024/2)
#define PAD         12
#define GOLD        0x53e5fc
#define GREEN       0x12cc12

typedef struct {
	BUF(int *batteries);
	BUF(int *highlights); // indices to highlight
} Bank;

void part_one(BUF(Bank *banks)) {
	int result = 0;

	for (int bank_i=0; bank_i<buf_len(banks); ++bank_i) {
		int max_num = 0;
		int max_index = 0;
		BUF(int *batteries) = banks[bank_i].batteries;
		int num_batteries = buf_len(batteries); 
		for (int i=0; i < num_batteries - 1; ++i) {
			if (batteries[i] > max_num) {
				max_num = batteries[i];
				max_index = i;
			}
		}

		int max_num_2 = 0;
		for (int i=max_index+1; i<buf_len(batteries); ++i) {
			if (batteries[i] > max_num_2) {
				max_num_2 = batteries[i];
			}
		}
		
		int joltage = (10 * max_num) + max_num_2;
		result += joltage;
	}
	
	printf("part one: %d\n", result);
}

void bank_max_in_range(Bank bank, int start_index, int stop_index, int *max_value, int *max_index) {
	int num_batteries = buf_len(bank.batteries);
	*max_value = 0;
	for (int i=start_index; i<stop_index; ++i) {
		if (bank.batteries[i] > *max_value) {
			*max_value = bank.batteries[i];
			*max_index = i;
		}
	}
}

U64 pow_ten(int exp) {
	U64 result = 1;
	for (int i=0; i<exp; ++i) result *= 10;
	return result;
}


void part_two(BUF(Bank *banks)) {
	U64 result = 0;

	for (int bank_i=0; bank_i<buf_len(banks); ++bank_i) {
		int max_index = -1;
		int max_value = 0;
		int num_batteries = buf_len(banks[bank_i].batteries);
		for (int digit = 12; digit > 0; --digit) {
			int start_index = max_index+1;
			bank_max_in_range(banks[bank_i], max_index+1, num_batteries - (digit - 1), &max_value, &max_index);
			result += pow_ten(digit-1) * max_value;
		}
	}

	printf("part two: %llu\n", result);
}

U32 colors[9] = {
	0x2a3328,
	0x404c3d,
	0x556651,
	0x6b7f66,
	0x80997a,
	0x95b28e,
	0xabcca3,
	0xc0e5b7,
	0xd6ffcc,
};

void draw_bank(Bank bank, int bank_index, int cell_size) {
	for (int i=0; i<buf_len(bank.batteries); ++i) {
		int x = PAD + i * cell_size;
		int y = PAD + bank_index * cell_size;
		drawrect(x, y, x+cell_size, y+cell_size, colors[bank.batteries[i] - 1]);
	}

	for (int i=0; i<buf_len(bank.highlights); ++i) {
		int x = PAD + bank.highlights[i] * cell_size;
		int y = PAD + bank_index * cell_size;
		strokerect(x, y, x+cell_size, y+cell_size, 1, GOLD);
	}
}

void draw_labels(int bank_index, int cell_size, U64 joltage, U64 sum) {
	int font_size = 4;
	int half_char_height = font_size * 12 / 2;
	int line_spacing = 2 * half_char_height;
	int x = SCREEN_MID + 16;
	int y = MAX(0, (bank_index * cell_size) - half_char_height);
	y = MIN(SCREEN_SIZE - 2*half_char_height - line_spacing, y);

	drawstringf(x, y, font_size, 0xffffff, "%15llu", joltage);
	drawstringf(x, y + line_spacing, font_size, GREEN, "%15llu", sum);
}


void visualize(BUF(Bank *banks)) {
	setupgif(1, 1, "lobby.gif");
	U64 result = 0;
	int cell_size = SCREEN_SIZE / buf_len(banks);

	for (int bank_i=0; bank_i<buf_len(banks); ++bank_i) {
		U64 joltage = 0;
		Bank bank = banks[bank_i];
		int max_index = -1;
		int max_value = 0;
		int num_batteries = buf_len(bank.batteries);
		for (int digit = 12; digit > 0; --digit) {
			int start_index = max_index+1;
			bank_max_in_range(bank, max_index+1, num_batteries - (digit - 1), &max_value, &max_index);
			joltage += pow_ten(digit-1) * max_value;
			buf_push(bank.highlights, max_index);
		}
		result += joltage;
		draw_bank(bank, bank_i, cell_size);
		drawrect(SCREEN_MID, 0, SCREEN_SIZE, SCREEN_SIZE, 0x000000);
		draw_labels(bank_i, cell_size, joltage, result);
		nextframe();
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

	BUF(Bank *banks) = NULL;

	for (;;) {
		Bank bank = {0};
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == 0) break;
		int len = (int)strlen(line);
		for (int i=0; i<len; ++i) {
			if (line[i] >= '1' && line[i] <= '9') {
				buf_push(bank.batteries, line[i] - '0');
			}
		}
		buf_push(banks, bank);
	}

	part_one(banks);
	part_two(banks);
	visualize(banks);

	return 0;
}
