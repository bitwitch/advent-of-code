#include "..\common.c"

#define MAX_WINNING  10
#define MAX_HAVE     25
#define MAX_CARDS   214

typedef struct {
	S32 card_num;
	S32 winning[MAX_WINNING];
	S32 num_winning;
	S32 have[MAX_HAVE];
	S32 num_have;
	S32 copies;
} Card;

static Card cards[MAX_CARDS];
static S32 num_cards;

static void part_one(void) {
	S32 sum = 0;
	for (S32 i=0; i<num_cards; ++i) {
		S32 points = 0;
		Card card = cards[i];
		for (S32 j=0; j<card.num_have; ++j) {
			S32 have = card.have[j];
			for (S32 k=0; k<card.num_winning; ++k) {
				S32 winning = card.winning[k];
				if (have == winning) {
					if (points == 0) {
						points = 1;
					} else {
						points *= 2;
					}
					break;
				}
			}	
		}	
		sum += points;
	}
	printf("Part one: %d\n", sum);
}

static void make_copies(S32 index, S32 count) {
	for (S32 copy_index = index + 1; copy_index <= index + count; ++copy_index) {
		++cards[copy_index].copies;
	}
}

static void part_two(void) {
	for (S32 card_index=0; card_index<num_cards; ++card_index) {
		Card *card = &cards[card_index];
		S32 matches = 0;
		for (S32 j=0; j<card->num_have; ++j) {
			S32 have = card->have[j];
			for (S32 k=0; k<card->num_winning; ++k) {
				S32 winning = card->winning[k];
				if (have == winning) {
					++matches;
					break;
				}
			}	
		}

		for (S32 i=0; i<card->copies+1; ++i) {
			make_copies(card_index, matches);
		}
	}

	S32 sum = 0;
	for (S32 i=0; i<num_cards; ++i) {
		Card *card = &cards[i];
		sum += 1 + card->copies;
	}

	printf("Part two: %d\n", sum);
}

static void parse_cards(char *input, U64 input_size) {
	while (match_token_name("Card")) {
		S32 card_num = (S32)parse_int();
		expect_token(':');

		Card card = {0};
		do {
			assert(card.num_winning < MAX_WINNING);
			card.winning[card.num_winning++] = (S32)parse_int();
		} while (!match_token('|'));
		do {
			assert(card.num_have < MAX_HAVE);
			card.have[card.num_have++] = (S32)parse_int();
		} while (!match_token('\n') && !is_token(TOKEN_EOF));

		assert(num_cards < MAX_CARDS);
		cards[num_cards++] = card;
	}
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
	parse_cards(file_data, file_size);
	printf("parsed %d cards\n", num_cards); 

	part_one();
	part_two();

	return 0;
}
