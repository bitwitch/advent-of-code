#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

typedef enum {
    OP_ADD,
    OP_MUL,
    OP_SQR,
    OP_COUNT,
} Operation;

typedef struct {
    U64 *items;
    Operation op;
    U64 operand;
    U64 divisor;
    U64 true_target, false_target;
    U64 items_inspected;
} Monkey;

void simulate(Monkey *monkeys, U64 turn_count, U64 worry_divisor, U64 common_divisor) {
    Monkey *m;
    U64 i, j, item, new, target, turns, count,
        max_one = 0, max_two = 0;

    for (turns=0; turns<turn_count; ++turns) {
        for (i=0; i<(U64)arrlen(monkeys); ++i) {
            m = &monkeys[i];
            for (j=0; j<(U64)arrlen(m->items); ++j) {
                m->items_inspected += 1;
                item = m->items[j];
                switch (m->op) {
                    case OP_ADD: new = item + m->operand; break;
                    case OP_MUL: new = item * m->operand; break;
                    case OP_SQR: new = item * item;       break;
                    default: 
                         fprintf(stderr, "Unrecognized op %d", m->op);
                         break;
                }
				new %= common_divisor;
                new /= worry_divisor;
                target = (new % m->divisor) == 0
                    ? m->true_target
                    : m->false_target;
                arrput(monkeys[target].items, new);
            }
            // clear the array
            arrsetlen(m->items, 0);
        }
    }

    for (i=0; i<(U64)arrlen(monkeys); ++i) {
        count = monkeys[i].items_inspected;

        if (count > max_one) {
            max_two = max_one;
            max_one = count;
        } else if (count > max_two) {
            max_two = count;
        }
    }

    printf("result: %llu\n", max_one * max_two);
}

Monkey *parse_monkeys(U8 *input, size_t input_size) {
    (void)input_size;
    Monkey *monkeys = NULL;

    char *line, *strnum;
    char c;

    do {
        Monkey monkey = {0};

        // Monkey N
        line = arb_chop_by_delimiter((char**)&input, "\n");

        // Starting items: 1,2,...
        line = arb_chop_by_delimiter((char**)&input, ": ");
        line = arb_chop_by_delimiter((char**)&input, "\n");
        do {
            strnum = arb_chop_by_delimiter(&line, ", ");
            arrput(monkey.items, atoi(strnum));
        } while (*line != '\0');

        // Operation: new = old * 13
        line = arb_chop_by_delimiter((char**)&input, "\n");
        sscanf(line, "  Operation: new = old %c %llu", &c, &monkey.operand);
        switch(c) {
            case '*': 
                if (strstr(line, "old * old"))
                    monkey.op = OP_SQR;
                else
                    monkey.op = OP_MUL;
                break;
            case '+': monkey.op = OP_ADD; break;
            default: 
                fprintf(stderr, "Unrecognized op %c\n", c);
                exit(1);
                break;
        }

        // Test: divisible by N
        line = arb_chop_by_delimiter((char**)&input, "\n"); 
        sscanf(line, "  Test: divisible by %llu", &monkey.divisor);

        // If true: throw to monkey N
        line = arb_chop_by_delimiter((char**)&input, "\n");
        sscanf(line, "    If true: throw to monkey %llu", &monkey.true_target);

        // If false: throw to monkey N
        line = arb_chop_by_delimiter((char**)&input, "\n");
        sscanf(line, "    If false: throw to monkey %llu", &monkey.false_target);

        line = arb_chop_by_delimiter((char**)&input, "\n");

        arrput(monkeys, monkey);
    } while (*input != '\0');

    return monkeys;
}

U64 get_common_divisor(Monkey *monkeys) {
	U64 i, common = 1;
	for (i=0; i<(U64)arrlen(monkeys); ++i)
		common *= monkeys[i].divisor;
	return common;
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

    Monkey *monkeys = parse_monkeys(file_data, file_size);

	U64 common_divisor = get_common_divisor(monkeys);


	// NOTE(shaw): only one of these can be uncommented at a time since
	// simulate modifies the monkeys

    /*simulate(monkeys, 20, 3, common_divisor);*/
    simulate(monkeys, 10000, 1, common_divisor);

    return 0;
}
