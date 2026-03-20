#include "..\common.c"
/*
   - only nums with even number of digits can contain repeats
   - does its first n digits match its last n digits
*/

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
	(void)ranges;
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

	return 0;
}
