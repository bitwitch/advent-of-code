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
    float x, y;
} Vec2f;

typedef struct {
    U64 worry;
    Vec2f start, end;
    float t; // lerp value
} Item;


typedef struct {
    U64 *items; // dynamic array of item handles
    Operation op;
    U64 operand;
    U64 divisor;
    U64 true_target, false_target;
    U64 items_inspected;
    Vec2f pos;
} Monkey;

/*while () {*/
    /*tick_sim(); */
    /*tick_visual();*/
    /*render();*/
/*}*/

global Item *items = NULL;

void tick_sim(Monkey *monkeys, U64 worry_divisor, U64 common_divisor, U64 *turn) {
    U64 i, j, new, target, item_handle;
    Monkey *m;
    Item *item;

    for (i=0; i<arrlenu(monkeys); ++i) {
        m = &monkeys[i];
        for (j=0; j<arrlenu(m->items); ++j) {
            m->items_inspected += 1;

            item_handle = m->items[j];
            item = &items[item_handle];

            // cant commit throw until monkey actually has item in hand
            /*if (item->t > 0 && item->t < 1) {*/
            /*return;*/
            /*}*/

            switch (m->op) {
                case OP_ADD: new = item->worry + m->operand; break;
                case OP_MUL: new = item->worry * m->operand; break;
                case OP_SQR: new = item->worry * item->worry; break;
                default: 
                             fprintf(stderr, "Unrecognized op %d", m->op);
                             break;
            }
            new %= common_divisor;
            new /= worry_divisor;
            target = (new % m->divisor == 0) ? m->true_target : m->false_target;

            item->worry = new;
            item->start = m->pos;
            item->end   = monkeys[target].pos;
            item->t     = 0;

            arrput(monkeys[target].items, item_handle);
        }
        // clear the array
        arrsetlen(m->items, 0);
    }
    *turn += 1;
}

void draw_monkeys(Monkey *monkeys) {
    int width=50, height=100;
    for (int i=0; i<arrlen(monkeys); ++i) {
        Monkey m = monkeys[i];
        drawbox(m.pos.x, m.pos.y, width, height, 0x375175);
    }
}

void tick_visual(Monkey *monkeys) {
    (void)monkeys;
}

void run(Monkey *monkeys, U64 turn_count, U64 worry_divisor, U64 common_divisor) {
    U64 i, count, turn=0, max_one=0, max_two=0;

    while (turn<turn_count) {
        tick_sim(monkeys, worry_divisor, common_divisor, &turn);
        tick_visual(monkeys);
    }

    for (i=0; i<arrlenu(monkeys); ++i) {
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
    int item_handle, i=0;

    do {
        Monkey monkey = {0};

        // Monkey N
        line = arb_chop_by_delimiter((char**)&input, "\n");

        // Starting items: 1,2,...
        line = arb_chop_by_delimiter((char**)&input, ": ");
        line = arb_chop_by_delimiter((char**)&input, "\n");
        do {
            Item item = {0};
            strnum = arb_chop_by_delimiter(&line, ", ");
            item.worry = atoi(strnum);
            item_handle = arrlen(items);
            arrput(items, item);
            arrput(monkey.items, item_handle);
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
        monkey.pos.x = 90 + 800 * 0.5 * (-cos(i * 0.7853981633974483f) + 1);
        monkey.pos.y = 512 - 50 - (300 * sin(i * 0.7853981633974483f));

        arrput(monkeys, monkey);
        ++i;
    } while (*input != '\0');

    return monkeys;
}

U64 get_common_divisor(Monkey *monkeys) {
	U64 i, common = 1;
	for (i=0; i<arrlenu(monkeys); ++i)
		common *= monkeys[i].divisor;
	return common;
}

void print_monkeys(Monkey *monkeys) {
    for (int i=0; i<arrlen(monkeys); ++i) {
        Monkey m = monkeys[i];
        printf("\n%d: operand=%llu divisor=%llu true=%llu false=%llu\n",
                i, m.operand, m.divisor, m.true_target, m.false_target);
        for (int j=0; j<arrlen(m.items); ++j) {
            printf("\t%llu  ", items[m.items[j]].worry);
        }
    }
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

    setupgif(0, 1, "monkey.gif");

   	// NOTE(shaw): only one of these can be uncommented at a time since
	// run modifies the monkeys

    /*run(monkeys, 20, 3, common_divisor);*/
    run(monkeys, 10000, 1, common_divisor);

    endgif();

    return 0;
}

