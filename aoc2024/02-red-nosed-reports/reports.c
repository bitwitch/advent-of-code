#include "..\common.c"
#include "..\lex.c"


void part_one(BUF(int **reports)) {
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

bool report_is_safe(BUF(int *report)) {
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

bool report_is_safe_remove_index(BUF(int *report), int index) {
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

void part_two(BUF(int **reports)) {
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

	return 0;
}
