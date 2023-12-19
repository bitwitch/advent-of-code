#include "..\common.c"

#define MAX_SEEDS 32

typedef struct {
	U64 src_start;
	U64 dst_start;
	U64 length;
} Range;

typedef enum {
	ELEMENT_SEED,
	ELEMENT_SOIL,
	ELEMENT_FERT,
	ELEMENT_WATER,
	ELEMENT_LIGHT,
	ELEMENT_TEMP,
	ELEMENT_HUMIDITY,
	ELEMENT_LOCATION,

	ELEMENT_COUNT,
} ElementKind;

typedef struct {
	ElementKind src;
	ElementKind dst;
	Range *ranges;
	int num_ranges;
} ElementMap; 

typedef struct {
	int num_seeds;
	U64 seeds[MAX_SEEDS];
	ElementMap maps[ELEMENT_COUNT];
} Almanac;

void parse_seeds(Almanac *almanac) {
	expect_token(TOKEN_NAME);
	expect_token(':');
	while (is_token(TOKEN_INT)) {
		assert(almanac->num_seeds < MAX_SEEDS);
		almanac->seeds[almanac->num_seeds++] = parse_int();
	}
	while (is_token('\n')) next_token();
}

void parse_element_map(Almanac *almanac, ElementKind src, ElementKind dst) {
	ElementMap *map = &almanac->maps[src];
	map->src = src;
	map->dst = dst;
	BUF(Range *ranges) = NULL;

	expect_token(TOKEN_NAME);
	expect_token('-');
	expect_token(TOKEN_NAME);
	expect_token('-');
	expect_token(TOKEN_NAME);
	expect_token(TOKEN_NAME);
	expect_token(':');
	expect_token('\n');

	while (is_token(TOKEN_INT)) {
		Range range = {0};
		range.dst_start = parse_int();
		range.src_start = parse_int();
		range.length = parse_int();
		buf_push(ranges, range);
		expect_token('\n');
	}

	while (is_token('\n')) next_token();

	map->ranges = ranges;
	map->num_ranges = buf_len(ranges);
}

Almanac *create_almanac(char *input) {
	Almanac *almanac = xcalloc(1, sizeof(Almanac));
	parse_seeds(almanac);
	for (int i=0; i<ELEMENT_COUNT-1; ++i) {
		parse_element_map(almanac, i, i+1);
	}
	return almanac;
}

// U64 soil_from_seed(Almanac *almanac, U64 seed) {

U64 transform_element(Almanac *almanac, U64 src_val, ElementKind src_kind) {
	U64 dst_val = src_val;
	ElementMap *src_map = &almanac->maps[src_kind];
	for (int i=0; i<src_map->num_ranges; ++i) {
		U64 len = src_map->ranges[i].length;
		U64 src_start = src_map->ranges[i].src_start;
		U64 dst_start = src_map->ranges[i].dst_start;
		if (src_val >= src_start && src_val <= src_start + len) {
			dst_val = (S64)(dst_start - src_start) + src_val;
			break;
		}
	}
	return dst_val;
}

U64 location_from_seed(Almanac *almanac, U64 seed) {
	U64 soil = transform_element(almanac, seed, ELEMENT_SEED);
	U64 fert = transform_element(almanac, soil, ELEMENT_SOIL);
	U64 water = transform_element(almanac, fert, ELEMENT_FERT);
	U64 light = transform_element(almanac, water, ELEMENT_WATER);
	U64 temp = transform_element(almanac, light, ELEMENT_LIGHT);
	U64 humidity = transform_element(almanac, temp, ELEMENT_TEMP);
	U64 location = transform_element(almanac, humidity, ELEMENT_HUMIDITY);
	return location;
}

void part_one(Almanac *almanac) {
	U64 min = UINT64_MAX;
	for (int i=0; i<almanac->num_seeds; ++i) {
		U64 location = location_from_seed(almanac, almanac->seeds[i]);
		if (location < min) min = location;
		printf("seed[%d] %llu -> %llu\n", i, almanac->seeds[i], location);
	}
	printf("Part one: %llu\n", min);
}

void part_two(Almanac *almanac) {
	(void)almanac;
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

	init_lexer(filename, file_data);
	Almanac *almanac = create_almanac(file_data);

	part_one(almanac);
	part_two(almanac);

	return 0;
}
