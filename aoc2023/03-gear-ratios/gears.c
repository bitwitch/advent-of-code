#include "..\common.c"
#include "..\..\base\sol_aoc.c"

#define GIF_WIDTH   1024
#define GIF_HEIGHT  1024
#define SKIP_FRAMES 1

typedef enum {
	STATE_NONE,
	STATE_REVEAL_KIND,
	STATE_REVEAL_GEARS,
} State;

typedef enum {
	CELL_EMPTY,
	CELL_NUMBER,
	CELL_SYMBOL,
} CellKind;

typedef struct {
	CellKind kind;
	int num;
	char sym;
	U32 color;
} Cell;

typedef struct {
	int rows, cols;
	Cell data[140*140];
} Schematic;


static U64 frame_count;
static U32 color_grey   = 0x333333;
static U32 color_white  = 0x696969;
static U32 color_red    = 0x1b13da;
static U32 color_green  = 0x4ba600;
static U32 color_yellow = 0x00d8f7;
static F64 dt;
static F64 last_timestamp;
static F64 countdown; 
static F64 cell_delay;
static Schematic *schematic;

static bool schematic_is_symbol(Schematic *schematic, int row, int col) {
	if (col < 0 || col >= schematic->cols) return false;
	if (row < 0 || row >= schematic->rows) return false;
	return schematic->data[row * schematic->cols + col].kind == CELL_SYMBOL;
}

static Cell *schematic_at(Schematic *schematic, int row, int col) {
	assert(col >= 0 && col < schematic->cols);
	assert(row >= 0 && row < schematic->rows);
	return &schematic->data[row * schematic->cols + col];
}

static bool schematic_is_gear(Schematic *schematic, int row, int col, int *ratio) {
	Cell *cell = schematic_at(schematic, row, col);
	if (cell->kind != CELL_SYMBOL) return false;
	if (cell->sym != '*') return false;

	int gear_nums[2];
	int nums_adjacent = 0;
	// check before and after
	cell = schematic_at(schematic, row, col-1);
	if (cell->kind == CELL_NUMBER) gear_nums[nums_adjacent++] = cell->num;
	cell = schematic_at(schematic, row, col+1);
	if (cell->kind == CELL_NUMBER) gear_nums[nums_adjacent++] = cell->num;

	// check above
	int last_num = 0;
	for (int i = col - 1; i <= col + 1; ++i) {
		cell = schematic_at(schematic, row-1, i);
		if (cell->kind == CELL_NUMBER && cell->num != last_num) {
			if (nums_adjacent > 2) return false;
			gear_nums[nums_adjacent++] = cell->num;
			last_num = cell->num;
		}
	}

	// check below
	last_num = 0;
	for (int i = col - 1; i <= col + 1; ++i) {
		cell = schematic_at(schematic, row+1, i);
		if (cell->kind == CELL_NUMBER && cell->num != last_num) {
			if (nums_adjacent > 2) return false;
			gear_nums[nums_adjacent++] = cell->num;
			last_num = cell->num;
		}
	}

	if (nums_adjacent == 2) {
		if (ratio) {
			*ratio = gear_nums[0] * gear_nums[1];
		}
		return true;
	}

	return false;
}

static int num_digits(int x) {
	int result = 0;
	do {
		++result;
		x /= 10;
	} while (x > 0);
	return result;
}

static bool schematic_is_part_number(Schematic *schematic, int row, int col) {
	Cell *cell = schematic_at(schematic, row, col);
	if (cell->kind != CELL_NUMBER) return false;

	// get first digit/col of number
	int prev_col = col - 1;
	while (prev_col >= 0) {
		Cell *previous_cell = schematic_at(schematic, row, prev_col);
		if (previous_cell->kind != CELL_NUMBER) break;
		cell = previous_cell;
		--col;
		--prev_col;
	}

	int width = num_digits(cell->num);

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

static void part_one(Schematic *schematic) {
	int sum = 0;
	for (int row=0; row < schematic->rows; ++row) {
		for (int col=0; col < schematic->cols; ++col) {
			Cell *cell = schematic_at(schematic, row, col);
			if (cell->kind == CELL_NUMBER) {
				if (schematic_is_part_number(schematic, row, col)) {
					sum += cell->num;
				}
				col += num_digits(cell->num) - 1;
			}
		}
	}
	printf("Part one: %d\n", sum);
}

static void part_two(Schematic *schematic) {
	int sum = 0;
	for (int row=0; row < schematic->rows; ++row) {
		for (int col=0; col < schematic->cols; ++col) {
			Cell *cell = schematic_at(schematic, row, col);
			if (cell->kind == CELL_SYMBOL && cell->sym == '*') {
				int ratio;
				if (schematic_is_gear(schematic, row, col, &ratio)) {
					sum += ratio;
				}
			}
		}
	}
	printf("Part two: %d\n", sum);
}

static Schematic *create_schematic(char *input) {
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

	// default color grey
	for (S32 i=0; i<count; ++i) {
		schematic->data[i].color = color_grey;
	}

	return schematic;
}

static void draw(void) {
	clear();

	F32 pad = 1;
	F32 available_width = GIF_WIDTH - ((schematic->cols + 1) * pad);
	F32 cell_size =  available_width / schematic->cols;

	for (S32 row=0; row<schematic->rows; ++row)
	for (S32 col=0; col<schematic->cols; ++col)
	{
		Cell *cell = schematic_at(schematic, row, col);
		int x = (int)(pad + col * (cell_size + pad));
		int y = (int)(pad + row * (cell_size + pad));
		drawbox(x, y, (int)cell_size, (int)cell_size, cell->color);
	}

	if (frame_count % SKIP_FRAMES == 0)
		nextframe();
	++frame_count;
}

static void animation(void) {
	draw();

	// reveal cell kind
	for (int row=0; row < schematic->rows; ++row) {
		for (int col=0; col < schematic->cols; ++col) {
			Cell *cell = schematic_at(schematic, row, col);
			if (schematic_is_part_number(schematic, row, col)) {
				cell->color = color_green;
			} else if (cell->kind == CELL_NUMBER) {
				cell->color = color_white;
			} else if (cell->kind == CELL_SYMBOL) {
				cell->color = color_yellow;
			}
		}
		draw();
	}

	// reveal cell gears
	for (int row=0; row < schematic->rows; ++row) {
		for (int col=0; col < schematic->cols; ++col)
		{
			Cell *cell = schematic_at(schematic, row, col);
			if (schematic_is_gear(schematic, row, col, NULL)) {
				cell->color = color_red;
			}
		}
		draw();
	}
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

	schematic = create_schematic(file_data);
	printf("input has %d cols and %d rows\n", schematic->cols, schematic->rows);

	part_one(schematic);
	part_two(schematic);

    setupgif(0, 2, "gears.gif");
	animation();
    endgif();

	return 0;
}


