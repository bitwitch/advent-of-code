#include "..\common.c"

void part_one(char *file_data, U64 file_size) {
	(void)file_data; 
	(void) file_size;
}

void part_two(char *file_data, U64 file_size) {
	(void)file_data; 
	(void) file_size;
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
