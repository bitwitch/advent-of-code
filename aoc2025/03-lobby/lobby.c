#include "..\common.c"

typedef struct {
	BUF(int *batteries);
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

void part_two(BUF(Bank *banks)) {
	int result = 0;
	// printf("part two: %d\n", result);
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

	return 0;
}
