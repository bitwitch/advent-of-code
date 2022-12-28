#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SKIP_FRAMES          3
#define THROW_DELAY_FRAMES   10
#define MAX_MONKEYS          256
#define MAX_NODES            256
#define MONKEY_WIDTH         50
#define MONKEY_HEIGHT        100

typedef enum {
    OP_ADD,
    OP_MUL,
    OP_SQR,
    OP_COUNT,
} Operation;

typedef struct {
    float x, y;
} Vec2f;

typedef struct Path_Node Path_Node;
struct Path_Node {
	int start;  // index of starting monkey
	int target; // index of target monkey
	Path_Node *next;
};

typedef struct {
    int worry;
	int delay;  // number of frames to wait before throwing
    Vec2f start, end;
    float t;    // lerp value
	Path_Node *path_head, *path_tail;
} Item;

typedef struct Node Node;
struct Node {
	Item item;
	Node *next;
};

typedef struct {
	Node *item_head, *item_tail;
    Operation op;
    int operand;
    int divisor;
    int true_target, false_target;
    U64 items_inspected;
    Vec2f pos;
    int throw_timer;
	int turn;
} Monkey;

global Node nodes[MAX_NODES];
global int node_count = 0;
global Monkey monkeys[MAX_MONKEYS];
global int monkey_count = 0;
global int frames = 0;

float lerpf(float a, float b, float t) {
	return a + (b-a)*t;
}

float vec2f_dist(Vec2f a, Vec2f b) {
	float x = fabs(a.x - b.x);
	float y = fabs(a.y - b.y);
	return sqrtf(x*x + y*y);
}

Vec2f vec2f_lerp(Vec2f a, Vec2f b, float t) {
	return (Vec2f){
		.x = lerpf(a.x, b.x, t),
		.y = lerpf(a.y, b.y, t)
	};
}

int list_count(Node *head) {
	int count = 0;
	for (Node *node = head;	node; node = node->next)
		++count;
	return count;
}

void draw_monkeys(Monkey *monkeys, int monkey_count, bool show_num_inspected) {
	for (int i=0; i<monkey_count; ++i) {
		Monkey m = monkeys[i];
		drawbox(m.pos.x, m.pos.y, MONKEY_WIDTH, MONKEY_HEIGHT, 0x375175);
		if (show_num_inspected)
			drawstringf(m.pos.x, m.pos.y - 25, 2, 0xFFFFFF, "%llu", m.items_inspected);
	}
}

void visualize_top_two(Monkey *monkeys, int monkey_count) {
	// get top two monkeys
	U64 result, count, max_one=0, max_two=0;
	int monkey_one, monkey_two;
	for (int i=0; i<monkey_count; ++i) {
		count = monkeys[i].items_inspected;
		if (count > max_one) {
			monkey_two = monkey_one;
			monkey_one = i;
			max_two = max_one;
			max_one = count;
		} else if (count > max_two) {
			max_two = count;
			monkey_two = i;
		}
	}

	result = max_one * max_two;
	printf("result: %llu\n", result);

	struct {
		Vec2f start, dest, pos;
		U64 inspected;
	} m1, m2;

	m1.start = (Vec2f){ monkeys[monkey_one].pos.x, monkeys[monkey_one].pos.y - 25 };
	m2.start = (Vec2f){ monkeys[monkey_two].pos.x, monkeys[monkey_two].pos.y - 25 };
	m1.dest  = (Vec2f){ 250, 512 };
	m2.dest  = (Vec2f){ 442, 512 };
	m1.inspected = monkeys[monkey_one].items_inspected;
	m2.inspected = monkeys[monkey_two].items_inspected;

	// glow the top two monkeys inspected count
	clear();
	draw_monkeys(monkeys, monkey_count, false);
	drawstringf(m1.start.x, m1.start.y, 2, 0x00FFFF, "%llu", m1.inspected);
	drawstringf(m2.start.x, m2.start.y, 2, 0x00FFFF, "%llu", m2.inspected);
	nextframe();
	nextframe();
	nextframe();
	nextframe();
	nextframe();

	// grow and move towards destination
	float step = 0.03;
	float t = 0;
	int scale = 2;
	while (t < 1) {
		clear();

		if (t < 0.33)
			draw_monkeys(monkeys, monkey_count, false);

		if (t > 0.33) scale = 3;
		if (t > 0.66) scale = 4;

		// monkey one 
		m1.pos = vec2f_lerp(m1.start, m1.dest, t);
		drawstringf(m1.pos.x, m1.pos.y, scale, 0x00FFFF, "%llu", m1.inspected);

		// monkey two 
		m2.pos = vec2f_lerp(m2.start, m2.dest, t);
		drawstringf(m2.pos.x, m2.pos.y, scale, 0x00FFFF, "%llu", m2.inspected);

		t += step;
		nextframe();
	}
	
	// draw result calculation
	clear();
	drawstringf(250, 512, 4, 0x00FFFF, "%llu * %llu = %llu", m1.inspected, m2.inspected, result);
	nextframe();
	nextframe();
}


void visualize(Node *nodes) {
	int i, half_size=5;
	U32 color = 0xFFFFFF;
	Item *item;

	// set initial path target for each item
	for (i=0; i<node_count; ++i) {
		item = &nodes[i].item;
		item->t = 0;
		if (item->path_head)
			item->end = monkeys[item->path_head->target].pos;
	}

	float step = 0.03;
	bool done = false;

	while (!done) {
		done = true;

		clear();
		draw_monkeys(monkeys, monkey_count, true);

		for (i=0; i<node_count; ++i) {
			item = &nodes[i].item;
			if (item->t < 1) {
				if (item->delay > 0) {
					--item->delay;
				} else {
					item->t = Clamp(0, item->t + step, 1);
				}
				done = false;
			} else if (item->path_head) {
				done = false;

				++monkeys[item->path_head->start].items_inspected;

				item->start = item->end;
				item->end = monkeys[item->path_head->target].pos;
				item->t = 0;
				SLLQueuePop(item->path_head, item->path_tail);
			}

			Vec2f pos = vec2f_lerp(item->start, item->end, item->t);
			drawbox(pos.x - half_size, 
					pos.y + MONKEY_HEIGHT/2 - half_size, 
					2*half_size, 2*half_size, color);
		}

		if (frames % SKIP_FRAMES == 0)
			nextframe();
		++frames;
	}
	nextframe();

	visualize_top_two(monkeys, monkey_count);
}

void parse_monkeys(U8 *input, size_t input_size) {
	(void)input_size;
	char *line, *strnum;
	char c;
	int i = 0;

	do {
		Monkey m = {0};

		// place monkeys in a circle and squash it a bit to fake some perspective
		m.pos.x = 90 + 800 * 0.5 * (-cos(i * 0.7853981633974483f) + 1);
		m.pos.y = 512 - 50 - (300 * sin(i * 0.7853981633974483f));

		// Monkey N
		line = arb_chop_by_delimiter((char**)&input, "\n");

		// Starting items: 1,2,...
		line = arb_chop_by_delimiter((char**)&input, ": ");
		line = arb_chop_by_delimiter((char**)&input, "\n");

		do {
			Node node = {0};
			node.item.start.x = node.item.end.x = m.pos.x;
			node.item.start.y = node.item.end.y = m.pos.y;
			strnum = arb_chop_by_delimiter(&line, ", ");
			node.item.worry = atoi(strnum);
			assert(node_count < MAX_NODES);
			nodes[node_count++] = node;
			SLLQueuePush(m.item_head, m.item_tail, &nodes[node_count - 1]);
		} while (*line != '\0');
		
		// Operation: new = old * 13
		line = arb_chop_by_delimiter((char**)&input, "\n");
		sscanf(line, "  Operation: new = old %c %d", &c, &m.operand);
		switch(c) {
			case '*': 
				if (strstr(line, "old * old"))
					m.op = OP_SQR;
				else
					m.op = OP_MUL;
				break;
			case '+': m.op = OP_ADD; break;
			default: 
				fprintf(stderr, "Unrecognized op %c\n", c);
				exit(1);
				break;
		}

		// Test: divisible by N
		line = arb_chop_by_delimiter((char**)&input, "\n"); 
		sscanf(line, "  Test: divisible by %d", &m.divisor);

		// If true: throw to monkey N
		line = arb_chop_by_delimiter((char**)&input, "\n");
		sscanf(line, "    If true: throw to monkey %d", &m.true_target);

		// If false: throw to monkey N
		line = arb_chop_by_delimiter((char**)&input, "\n");
		sscanf(line, "    If false: throw to monkey %d", &m.false_target);
		line = arb_chop_by_delimiter((char**)&input, "\n");

		assert(monkey_count < MAX_MONKEYS);
		monkeys[monkey_count++] = m;

		++i;
	} while (*input != '\0');
}

int get_common_divisor(Monkey *monkeys, int monkey_count) {
	int i, common = 1;
	for (i=0; i<monkey_count; ++i)
		common *= monkeys[i].divisor;
	return common;
}

void print_items(void) {
	for (int j=0; j<node_count; ++j) {
		Item i = nodes[j].item;
		printf("worry=%d start=(%f,%f) end=(%f,%f) t=%f\n", 
				i.worry, i.start.x, i.start.y, i.end.x, i.end.y, i.t);
	}
}

void print_monkey(Monkey m) {
	printf("operand=%d divisor=%d true=%d false=%d\n",
		m.operand, m.divisor, m.true_target, m.false_target);
	for (Node *node = m.item_head; node != NULL; node = node->next) {
		printf("  %d", node->item.worry);
	}
	printf("\n");
}

void print_monkeys(Monkey *monkeys) {
	for (int i=0; i<monkey_count; ++i) {
		printf("Monkey %d: ", i);
		print_monkey(monkeys[i]);
	}
}

void calculate_item_paths(Monkey *monkeys, int monkey_count, int turn_count, int worry_divisor, int common_divisor) {
    Monkey *m;
	int turn, i, target;
    U64 new;
	Node *node;
	Path_Node *path_node;
	Item *item;
	U64 count, max_one=0, max_two=0;

    for (turn=0; turn<turn_count; ++turn) {
        for (i=0; i<monkey_count; ++i) {
			int throw_delay = 0;
            m = &monkeys[i];
			while (m->item_head) {
				node = m->item_head;
				item = &node->item;

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
				item->worry = new;

				item->delay = throw_delay;
				throw_delay += THROW_DELAY_FRAMES;

				target = (new % m->divisor == 0) ? m->true_target : m->false_target;

				// throw from m to monkeys[target]
				SLLQueuePop(m->item_head, m->item_tail);
				node->next = NULL;
				SLLQueuePush(monkeys[target].item_head, monkeys[target].item_tail, node);
				m->items_inspected += 1;
				
				// add to path of item
				path_node = malloc(sizeof(Path_Node));
				if (!path_node) { perror("malloc"); exit(1); }
				path_node->start = i;
				path_node->target = target;
				SLLQueuePush(item->path_head, item->path_tail, path_node);
            }
        }
    }

	for (i=0; i<monkey_count; ++i) {
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

	parse_monkeys(file_data, file_size);

	int common_divisor = get_common_divisor(monkeys, monkey_count);

	int turn_count = 20;
	int worry_divisor = 3;
	/*int turn_count = 10000;*/
	/*int worry_divisor = 1;*/

	calculate_item_paths(monkeys, monkey_count, turn_count, worry_divisor, common_divisor);

	// reset number of items inspected so the visualization can show them counting up
	for (int i=0; i<monkey_count; ++i) {
		monkeys[i].items_inspected = 0;
	}

	setupgif(0, 1, "monkey.gif");
	visualize(nodes);
	endgif();

	return 0;
}
