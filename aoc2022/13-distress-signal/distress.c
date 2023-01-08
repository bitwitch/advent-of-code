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
    union {
        int num;
        Element *list; // dynamic array
    } data;
};

void print_list(Element *list) {
	printf("[ ");
	for (int i=0; i<arrlen(list); ++i) {
		Element e = list[i];
		if (e.type == ETYPE_INT)
			printf("%d, ", e.data.num);
		else
			print_list(e.data.list);
	}
	printf("], ");
}

int compare_elements(Element left, Element right) {
	int result = 0;

	if (left.type == ETYPE_INT && right.type == ETYPE_INT) {
		result = left.data.num - right.data.num;

	} else if (left.type == ETYPE_LIST && right.type == ETYPE_LIST) {
		int i_left=0, i_right=0;
		while (i_left < arrlen(left.data.list) && i_right < arrlen(right.data.list)) {
			result = compare_elements(left.data.list[i_left], right.data.list[i_right]);
			if (result != 0) break;
			++i_left; ++i_right;
		}
		if (result == 0) {
			result = arrlen(left.data.list) - arrlen(right.data.list);
		}

	} else { 
		Element as_list = { .type = ETYPE_LIST };
		if (left.type == ETYPE_LIST) {
			arrput(as_list.data.list, right);
			result = compare_elements(left, as_list);
		} else {
			arrput(as_list.data.list, left);
			result = compare_elements(as_list, right);
		}
		arrfree(as_list.data.list);
	}
	return result;
}

void part_one(Element pairs) {
	int pair_index = 1;
	int sum = 0;
	for (int i=0; i<arrlen(pairs.data.list) - 1; i+=2) {
		Element left = pairs.data.list[i];		
		Element right = pairs.data.list[i+1];		
		int result = compare_elements(left, right);
		if (result <= 0) sum += pair_index;
		++pair_index;
	}
	printf("part_one: %d\n", sum);
}

void part_two(Element pairs) {
	(void)pairs;
    printf("part_two: %d\n", 696969);
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
			arrput(e.data.list, child);
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
		e.data.num = atoi(start);
	}

	return e;
}

Element parse_input(U8 *input) {
	Element pairs = { .type = ETYPE_LIST };

	do {
		char *line = arb_chop_by_delimiter((char**)&input, "\n");
		if (*line == '\0') continue;
		Element current = parse_element(&line);
		arrput(pairs.data.list, current);
	} while (*input != '\0');

	return pairs;
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

	Element pairs = parse_input(file_data);

    part_one(pairs);
    part_two(pairs);

    return 0;
}
