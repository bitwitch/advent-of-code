#include <math.h> // for pow
#include "../common.c"
#include "../lex.c"

typedef struct Node Node;
struct Node {
	U64 num;
	Node *next;
};

typedef struct {
	Node *first;
	Node *last;
	U64 count;
} List;

Arena arena = {0};

void list_push(List *list, U64 num) {
	Node *node = arena_alloc(&arena, sizeof(Node));
	node->num = num;
	node->next = NULL;
	if (list->first == NULL) {
		list->first = node;
		list->last = node;
	} else {
		list->last->next = node;
		list->last = node;
	}
	++list->count;
}

void list_insert_after(List *list, Node *node, U64 num) {
	Node *new = arena_alloc(&arena, sizeof(Node));
	new->num = num;
	new->next = node->next;
	node->next = new;
	if (list->last == node) {
		list->last = new;
	}
	++list->count;
}

void list_print(List *list) {
	for (Node *n = list->first; n; n = n->next) {
		printf("%llu ", n->num);
	}
	printf("\n");
}

int num_digits(U64 num) {
	if (num == 0) return 1;
	int result = 0;
	while (num) {
		num /= 10;
		++result;
	}
	return result;
}

bool even_digits(U64 num) {
	return num_digits(num) % 2 == 0;
}

U64 blink(List *pebbles, int num_blinks) {
	for (int i=0; i<num_blinks; ++i) {
		printf("blink %d\n", i);
		if (i % 10 == 0) {
			printf("\t%llu pebbles\n", pebbles->count);
		}
		for (Node *n = pebbles->first; n; n = n->next) {
			if (n->num == 0) {
				n->num = 1;
			} else if (even_digits(n->num)) {
				int half_digits = num_digits(n->num) / 2;
				U64 divisor = (U64)pow(10, half_digits);
				U64 left_half = n->num / divisor;
				U64 right_half = n->num % divisor;
				n->num = left_half;
				list_insert_after(pebbles, n, right_half);
				n = n->next;
			} else {
				n->num *= 2024;
			}
		}
	}

	U64 num_stones = 0;
	for (Node *n = pebbles->first; n; n = n->next) {
		++num_stones;
	}
	return num_stones;
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

	arena_grow(&arena, 500000 * sizeof(Node));

	List pebbles1 = {0};
	List pebbles2 = {0};

	while (!is_token(TOKEN_EOF)) {
		if (is_token(TOKEN_INT)) {
			int n = parse_int();
			list_push(&pebbles1, n);
			list_push(&pebbles2, n);
		} else {
			next_token();
		}
	}

	U64 part_one = blink(&pebbles1, 25);
	printf("part one: %llu\n", part_one);

	U64 part_two = blink(&pebbles2, 75);
	printf("part two: %llu\n", part_two);

	// U64 part_two = blink_buf(25);
	// printf("part two: %llu\n", part_two);



	return 0;
}

