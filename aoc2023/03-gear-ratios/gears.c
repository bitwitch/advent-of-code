#include "..\common.c"

typedef enum {
	CELL_EMPTY,
	CELL_NUMBER,
	CELL_SYMBOL,
} CellKind;

typedef struct {
	CellKind kind;
	int num;
	char sym;
} Cell;

typedef struct {
	int rows, cols;
	Cell data[142*142];
} Schematic;

bool schematic_is_symbol(Schematic *schematic, int row, int col) {
	if (col < 0 || col >= schematic->cols) return false;
	if (row < 0 || row >= schematic->rows) return false;
	return schematic->data[row * schematic->cols + col].kind == CELL_SYMBOL;
}

Cell schematic_at(Schematic *schematic, int row, int col) {
	assert(col >= 0 && col < schematic->cols);
	assert(row >= 0 && row < schematic->rows);
	return schematic->data[row * schematic->cols + col];
}

bool schematic_is_gear(Schematic *schematic, int row, int col, int *ratio) {
	Cell cell = schematic_at(schematic, row, col);
	if (cell.kind != CELL_SYMBOL) return false;
	if (cell.sym != '*') return false;

	int gear_nums[2];
	int nums_adjacent = 0;
	// check before and after
	cell = schematic_at(schematic, row, col-1);
	if (cell.kind == CELL_NUMBER) gear_nums[nums_adjacent++] = cell.num;
	cell = schematic_at(schematic, row, col+1);
	if (cell.kind == CELL_NUMBER) gear_nums[nums_adjacent++] = cell.num;

	// check above
	int last_num = 0;
	for (int i = col - 1; i <= col + 1; ++i) {
		cell = schematic_at(schematic, row-1, i);
		if (cell.kind == CELL_NUMBER && cell.num != last_num) {
			if (nums_adjacent > 2) return false;
			gear_nums[nums_adjacent++] = cell.num;
			last_num = cell.num;
		}
	}

	// check below
	last_num = 0;
	for (int i = col - 1; i <= col + 1; ++i) {
		cell = schematic_at(schematic, row+1, i);
		if (cell.kind == CELL_NUMBER && cell.num != last_num) {
			if (nums_adjacent > 2) return false;
			gear_nums[nums_adjacent++] = cell.num;
			last_num = cell.num;
		}
	}

	if (nums_adjacent == 2) {
		*ratio = gear_nums[0] * gear_nums[1];
		return true;
	}

	return false;
}

int num_digits(int x) {
	int result = 0;
	do {
		++result;
		x /= 10;
	} while (x > 0);
	return result;
}

// assumes it is given the first col of a number
bool is_part_number(Schematic *schematic, int row, int col) {
	Cell cell = schematic_at(schematic, row, col);
	assert(cell.kind == CELL_NUMBER);
	int width = num_digits(cell.num);

	// check before and after
	if (schematic_is_symbol(schematic, row, col-1)) return true;
	if (schematic_is_symbol(schematic, row, col+width)) return true;
	// check above and below
	for (int i = col - 1; i <= col + width; ++i) {
		if (schematic_is_symbol(schematic, row-1, i)) return true;
		if (schematic_is_symbol(schematic, row+1, i)) return true;
	}
	return false;
}

void part_one(Schematic *schematic) {
	int sum = 0;
	for (int row=0; row < schematic->rows; ++row) {
		for (int col=0; col < schematic->cols; ++col) {
			Cell cell = schematic_at(schematic, row, col);
			if (cell.kind == CELL_NUMBER) {
				if (is_part_number(schematic, row, col)) {
					sum += cell.num;
				}
				col += num_digits(cell.num) - 1;
			}
		}
	}
	printf("Part one: %d\n", sum);
}

void part_two(Schematic *schematic) {
	int sum = 0;
	for (int row=0; row < schematic->rows; ++row) {
		for (int col=0; col < schematic->cols; ++col) {
			Cell cell = schematic_at(schematic, row, col);
			if (cell.kind == CELL_SYMBOL && cell.sym == '*') {
				int ratio;
				if (schematic_is_gear(schematic, row, col, &ratio)) {
					sum += ratio;
				}
			}
		}
	}
	printf("Part two: %d\n", sum);
}

Schematic *create_schematic(char *input) {
	Schematic *schematic = xcalloc(1, sizeof(Schematic));
	int count = 0;

	int cols = 0;
	while (input[cols] != '\n' && input[cols] != '\r') {
		++cols; 
	}
	schematic->cols = cols;

	do {
		char *line = chop_by_delimiter(&input, "\n");
		++schematic->rows;

		while (*line != '\0') {
			if (*line == '.') {
				++line;
				++count;
			} else if (isdigit(*line)) {
				char *start = line;
				int num = strtol(line, &line, 10);
				while (start != line) {
					schematic->data[count++] = (Cell){
						.kind = CELL_NUMBER,
						.num = num	
					};
					++start;
				}
			} else if (*line == '\r') {
				++line;
			} else {
				schematic->data[count++] = (Cell) {
					.kind = CELL_SYMBOL,
					.sym = *line,
				};
				++line;
			}
		}
	} while (*input != '\0');

	return schematic;
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

	Schematic *schematic = create_schematic(file_data);
	printf("input has %d cols and %d rows\n", schematic->cols, schematic->rows);

	part_one(schematic);
	part_two(schematic);

	return 0;
}


