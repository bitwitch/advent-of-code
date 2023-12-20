#include "..\common.c"

#define MAX_RECORDS 4

typedef struct {
	int times[MAX_RECORDS];
	int distances[MAX_RECORDS];
	int num_records;
} RaceData;

void part_one(RaceData *race_data) {
	U64 product = 1;
	for (int i=0; i<race_data->num_records; ++i) {
		int sum = 0;
		int time = race_data->times[i];
		int record_dist = race_data->distances[i];

		for (int hold=1; hold<time; ++hold) {
			int dist = hold * (time - hold);
			if (dist > record_dist) {
				++sum;
			}
		}
		product *= sum;
	}
	printf("Part one: %llu\n", product);
}

void parse_part_two(char *input, U64 *time, U64 *record_dist) {
	// get time
	while (!isdigit(*input)) ++input;
	char time_buf[32] = {0};
	char buf_index = 0;
	while (isdigit(*input)) {
		assert(buf_index < ARRAY_COUNT(time_buf) - 1);
		time_buf[buf_index++] = *input;
		++input;
		while (*input == ' ') ++input;
	}
	*time = strtoll(time_buf, NULL, 10);

	// get distance
	while (!isdigit(*input)) ++input;
	char dist_buf[32] = {0};
	buf_index = 0;
	while (isdigit(*input)) {
		assert(buf_index < ARRAY_COUNT(dist_buf) - 1);
		dist_buf[buf_index++] = *input;
		++input;
		while (*input == ' ') ++input;
	}
	*record_dist = strtoll(dist_buf, NULL, 10);
}

void part_two(char *input) {
	U64 time, record_dist;
	parse_part_two(input, &time, &record_dist);

	U64 sum = 0;
	U64 half = (time - 1) / 2;

	for (U64 hold=1; hold<=half; ++hold) {
		U64 dist = hold * (time - hold);
		if (dist > record_dist) {
			sum += 2;
		}
	}

	// check middle if possible hold values are odd (time is even)
	if ((time % 2) == 0) {
		U64 hold = half + 1;	
		U64 dist = hold * (time - hold);
		if (dist > record_dist) {
			++sum;
		}
	}

	printf("Part two: %llu\n", sum);
}

RaceData parse_race_data(void) {
	RaceData results = {0};
	expect_token(TOKEN_NAME);
	expect_token(':');
	while (is_token(TOKEN_INT)) {
		assert(results.num_records < MAX_RECORDS);
		results.times[results.num_records++] = (int)parse_int();
	}

	expect_token('\n');
	expect_token(TOKEN_NAME);
	expect_token(':');
	int i = 0;
	while (is_token(TOKEN_INT)) {
		assert(i < MAX_RECORDS);
		results.distances[i++] = (int)parse_int();
	}

	return results;
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
	RaceData race_data = parse_race_data();
	part_one(&race_data);
	part_two(file_data);

	return 0;
}
