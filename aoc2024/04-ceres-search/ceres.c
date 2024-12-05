#include "..\common.c"
#include "..\lex.c"

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

	init_stream(file_name, file_data);
	BUF(char **grid) = NULL;

	while (!is_token(TOKEN_EOF)) {

		do {
			if (is_token(TOKEN_NAME)) {
			}
			BUF(char *row) = NULL;
			while (*cursor != '\n') {
			char level = (char)parse_int();
			buf_push(report, level);
		} while (!match_token('\n'));
		buf_push(reports, report);
	}

	part_one(reports);
	part_two(reports);

	return 0;
}

