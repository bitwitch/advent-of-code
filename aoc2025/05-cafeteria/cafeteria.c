#include "..\common.c"

typedef struct {
	U64 min, max;
} Range;

typedef struct {
	BUF(Range *fresh);
	BUF(U64 *ingredients);
} Database;

void part_one(Database db) {
	int result = 0;
	for (int i=0; i<buf_len(db.ingredients); ++i) {
		U64 id = db.ingredients[i];
		for (int j=0; j<buf_len(db.fresh); ++j) {
			Range range = db.fresh[j];
			if (id >= range.min && id <= range.max) {
				result += 1;
				break;
			}
		}
	}
	printf("part one: %d\n", result);
}

void part_two(Database db) {
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

	Database db = {0};

	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == 0 || *line == '\r') break;
		Range range = {0};
		if (sscanf(line, "%llu-%llu", &range.min, &range.max) != 2) {
			fatal("failed to parse range: %s", line);
		}
		buf_push(db.fresh, range);
	}

	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == 0 || *line == '\r') break;
		U64 id = 0;
		if (sscanf(line, "%llu", &id) != 1) {
			fatal("failed to parse ingredient id: %s", line);
		}
		buf_push(db.ingredients, id);
	}

	part_one(db);
	part_two(db);

	return 0;
}
