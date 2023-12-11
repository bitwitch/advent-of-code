#include "..\common.c"
#include <ctype.h> // for isdigit()

int digit_from_char(char c) {
	return c - 48;
}

void part_one(char *file_data, U64 file_size) {
	(void)file_size;

	char *cursor = file_data;
	U64 sum = 0;

	while (true) {
		while (!isdigit(*cursor)) {
			if (*cursor == '\0') {
				goto done;
			}
			++cursor;
		}
		U64 digit0 = digit_from_char(*cursor);

		U64 digit1 = 0;
		while (*cursor != '\n') {
			if (*cursor == '\0') {
				goto done;
			}
			if (isdigit(*cursor)) {
				digit1 = digit_from_char(*cursor);
			}
			++cursor;
		}
		++cursor;
		U64 number = 10 * digit0 + digit1;
		sum += number;
	}
done:
	printf("Part One: %zu\n", sum);
}

int parse_digit(char *s) {
	if (isdigit(*s))
		return digit_from_char(*s);

	if (0 == strncmp(s, "one", 3)) return 1;
	if (0 == strncmp(s, "two", 3)) return 2;
	if (0 == strncmp(s, "three", 5)) return 3;
	if (0 == strncmp(s, "four", 4)) return 4;
	if (0 == strncmp(s, "five", 4)) return 5;
	if (0 == strncmp(s, "six", 3)) return 6;
	if (0 == strncmp(s, "seven", 5)) return 7;
	if (0 == strncmp(s, "eight", 5)) return 8;
	if (0 == strncmp(s, "nine", 4)) return 9;

	return -1;
}

void part_two(char *file_data, U64 file_size) {
	(void)file_size;

	char *cursor = file_data;
	U64 sum = 0;

	while (true) {

		U64 val = parse_digit(cursor);
		if (*cursor == '\0') {
			goto done;
		}
		while (val == -1) {
			val = parse_digit(cursor);
			if (*cursor == '\0') {
				goto done;
			}
			++cursor;
		}
		U64 digit0 = val;

		U64 digit1 = val;
		while (*cursor != '\n') {
			if (*cursor == '\0') {
				goto done;
			}
			val = parse_digit(cursor);
			if (val != -1) {
				digit1 = val;
			}
			++cursor;
		}
		++cursor;
		U64 number = 10 * digit0 + digit1;
		sum += number;
	}
done:
	printf("Part Two: %zu\n", sum);
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

	part_one(file_data, file_size);
	part_two(file_data, file_size);

	return 0;
}
