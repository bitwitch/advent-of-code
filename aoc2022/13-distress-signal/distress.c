#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

typedef enum {
    ETYPE_INT, ETYPE_LIST
} Etype;

// TODO(shaw): Element is a terrible name, but idk what else to use
typedef struct Element Element;
struct Element {
    Etype type;
	int num;
	Element *list; // dynamic array
};

void print_list(Element *list) {
	printf("[ ");
	for (int i=0; i<arrlen(list); ++i) {
		Element e = list[i];
		if (e.type == ETYPE_INT)
			printf("%d, ", e.num);
		else
			print_list(e.list);
	}
	printf("], ");
}

int compare_elements(Element left, Element right) {
	int result = 0;

	if (left.type == ETYPE_INT && right.type == ETYPE_INT) {
		result = left.num - right.num;

	} else if (left.type == ETYPE_LIST && right.type == ETYPE_LIST) {
		int i_left=0, i_right=0;
		while (i_left < arrlen(left.list) && i_right < arrlen(right.list)) {
			result = compare_elements(left.list[i_left], right.list[i_right]);
			if (result != 0) break;
			++i_left; ++i_right;
		}
		if (result == 0) {
			result = arrlen(left.list) - arrlen(right.list);
		}

	} else { 
		Element as_list = { .type = ETYPE_LIST };
		if (left.type == ETYPE_LIST) {
			arrput(as_list.list, right);
			result = compare_elements(left, as_list);
		} else {
			arrput(as_list.list, left);
			result = compare_elements(as_list, right);
		}
		arrfree(as_list.list);
	}
	return result;
}

int qsort_compare_elements(const void *a, const void *b) {
	Element left = *((Element*)a);
	Element right = *((Element*)b);
	return compare_elements(left, right);
}

void part_one(Element packets) {
	int pair_index = 1;
	int sum = 0;
	for (int i=0; i<arrlen(packets.list) - 1; i+=2) {
		Element left = packets.list[i];		
		Element right = packets.list[i+1];		
		int result = compare_elements(left, right);
		if (result <= 0) sum += pair_index;
		++pair_index;
	}
	printf("part_one: %d\n", sum);
}

void part_two(Element packets) {
	// add divider packets
	Element divider_one = { .type = ETYPE_LIST };
	Element d1_list = { .type = ETYPE_LIST };
	Element d1_num = { .type = ETYPE_INT, .num = 2 };
	arrput(d1_list.list, d1_num);
	arrput(divider_one.list, d1_list);

	Element divider_two = { .type = ETYPE_LIST };
	Element d2_list = { .type = ETYPE_LIST };
	Element d2_num = { .type = ETYPE_INT, .num = 6 };
	arrput(d2_list.list, d2_num);
	arrput(divider_two.list, d2_list);

	arrput(packets.list, divider_one);
	arrput(packets.list, divider_two);

	// sort packets
	qsort(packets.list, arrlen(packets.list), sizeof(Element), qsort_compare_elements);

	// locate divider packets
	int index_one=0, index_two=0; 
	for (int i=0; i<arrlen(packets.list); ++i) {
		Element p = packets.list[i];
		if (arrlen(p.list) != 1) continue;
		Element first = p.list[0];
		if (first.type == ETYPE_LIST && arrlen(first.list) == 1) {
			if (first.list[0].num == 2)
				index_one = i+1;
			if (first.list[0].num == 6)
				index_two = i+1;
		}
	}

	printf("part_two: %d\n", index_one * index_two);
}

Element parse_element(char **line) {
	Element e = {0};

	// skip commas and spaces
	while (*(*line) == ' ' || *(*line) == ',') {
		++(*line);
	}

	if (*(*line) == '[') {
		++(*line);
		e.type = ETYPE_LIST;
		while (*(*line) != ']') {
			Element child = parse_element(line);
			arrput(e.list, child);
		}
		++(*line); // eat closing ]

	} else { // number
		e.type = ETYPE_INT;
		char *start = *line;
		while (*(*line) >= '0' && *(*line) <= '9') {
			++(*line);
		}
		// NOTE(shaw): atoi converts the initial portion of the string pointed
		// to by nptr to int, meaning it will stop parsing the number when it
		// reaches a non base 10 digit
		e.num = atoi(start);
	}

	return e;
}

Element parse_input(U8 *input) {
	Element packets = { .type = ETYPE_LIST };

	do {
		char *line = arb_chop_by_delimiter((char**)&input, "\n");
		if (*line == '\0') continue;
		Element current = parse_element(&line);
		arrput(packets.list, current);
	} while (*input != '\0');

	return packets;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Must supply filepath to puzzle input\n");
        exit(1);
    }

    FILE *fp;
    U8 *file_data;
    size_t file_size;

    fp = fopen(argv[1], "r");
    if (!fp) { perror("fopen"); exit(1); }

    int ok = arb_read_entire_file(fp, &file_data, &file_size);
    if (ok != ARB_READ_ENTIRE_FILE_OK) {
        fprintf(stderr, "Error: arb_read_entire_file()\n");
        fclose(fp);
        exit(1);
    }

    fclose(fp);

	Element packets = parse_input(file_data);

    part_one(packets);
    part_two(packets);

    return 0;
}
