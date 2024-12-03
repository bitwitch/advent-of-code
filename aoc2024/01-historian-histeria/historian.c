#include "..\common.c"
#include "..\lex.c"

int compare_s64(const void* a, const void* b) {
    S64 arg1 = *(const S64*)a;
    S64 arg2 = *(const S64*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void part_one(S64 *nums1, S64 *nums2) {
	U64 sum = 0;
	
	assert(buf_len(nums1) == buf_len(nums2));
	for (S64 i=0; i<buf_len(nums1); ++i) {
		S64 diff = llabs(nums1[i] - nums2[i]);
		sum += diff;
	}

	printf("part one: %llu\n", sum);
}

void part_two(S64 *nums1, S64 *nums2) {
	Map frequency = {0};

	for (S64 i=0; i<buf_len(nums2); ++i) {
		S64 n = nums2[i];
		U64 count = (U64)map_get(&frequency, (void*)n);
		map_put(&frequency, (void*)n, (void*)(count + 1));
	}

	U64 similarity = 0;
	for (S64 i=0; i<buf_len(nums1); ++i) {
		S64 n = nums1[i];
		U64 count = (U64)map_get(&frequency, (void*)n);
		similarity += n * count;
	}

	printf("part two: %llu\n", similarity);
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

	BUF(S64 *nums1) = NULL;
	BUF(S64 *nums2) = NULL;

	while (!is_token(TOKEN_EOF)) {
		S64 n1 = (int)parse_int();
		S64 n2 = (int)parse_int();
		if (is_token('\n')) {
			next_token();
		}
		buf_push(nums1, n1);
		buf_push(nums2, n2);
	}

	qsort(nums1, buf_len(nums1), sizeof(S64), compare_s64);
	qsort(nums2, buf_len(nums2), sizeof(S64), compare_s64);

	part_one(nums1, nums2);
	part_two(nums1, nums2);

	return 0;
}
