#include "../common.c"
#include "../lex.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE   1024
#define CELL_SIZE     15
#define CELL_PAD      1
#define MACHINE_PAD_Y 5
#define MACHINE_PAD_X (12 * CELL_SIZE + 100)
#define GREEN_ON      0x12f412
#define GREEN_OFF     0x063d06


typedef struct {
	BUF(int *ids); 
} Button;

typedef struct {
	bool on;
	int joltage;
} Light;

typedef struct {
	int frame;
	int skip_frames;
	int min_presses;
	BUF(Button *buttons);
	BUF(Light *light_schema);
	BUF(Light *lights);
	BUF(bool *light_frames);
} Machine;

bool is_valid_light(Machine m, int *button_ids, int num_ids) {
	for (int i=0; i<num_ids; ++i) {
		Button button = m.buttons[button_ids[i]];
		for (int j=0; j<buf_len(button.ids); ++j) {
			int id = button.ids[j];
			m.lights[id].on = !m.lights[id].on;
		}
	}	
	for (int i=0; i<buf_len(m.light_schema); ++i) {
		if (m.lights[i].on != m.light_schema[i].on) return false;
	}
	return true;
}

bool is_valid_joltage(Machine m, int *button_ids, int num_ids) {
	for (int i=0; i<num_ids; ++i) {
		Button button = m.buttons[button_ids[i]];
		for (int j=0; j<buf_len(button.ids); ++j) {
			int id = button.ids[j];
			m.lights[id].joltage += 1;
		}
	}	
	for (int i=0; i<buf_len(m.light_schema); ++i) {
		if (m.lights[i].joltage != m.light_schema[i].joltage) return false;
	}
	return true;
}


typedef bool (*ValidationFunc)(Machine, int *, int); 

bool permute(Machine machine, int n, int k, int depth, int *button_ids, ValidationFunc validate) {
	if (depth == k) {
		bool valid = validate(machine, button_ids, k);
		buf_zero(machine.lights);
		return valid;
	}

	for (int i=0; i<n; ++i) {
		button_ids[depth] = i;
		if (permute(machine, n, k, depth+1, button_ids, validate)) {
			return true;
		}
	}
	return false;
}

void part_one(BUF(Machine *machines)) {
	int result = 0;
	BUF(int *button_ids) = NULL;

	for (int machine_i = 0; machine_i < buf_len(machines); ++machine_i) {
		Machine m = machines[machine_i];
		bool found_valid = false;
		for (int choose=1; choose<=10; ++choose) {
			buf_set_len(button_ids, choose);
			buf_zero(button_ids);
			if (permute(m, buf_len(m.buttons), choose, 0, button_ids, is_valid_light)) {
				result += choose;
				found_valid = true;
				break;
			}
		}
		if (!found_valid) {
			printf("failed to find initialization for machine[%d]\n", machine_i);
		}
	}

	printf("part one: %d\n", result);
}

void part_two(BUF(Machine *machines)) {
	int result = 0;
	BUF(int *button_ids) = NULL;

	for (int machine_i = 0; machine_i < buf_len(machines); ++machine_i) {
		printf("Machine %d\n", machine_i);
		Machine m = machines[machine_i];
		bool found_valid = false;
		for (int choose=1; choose<=12; ++choose) {
			printf("                    \r    choosing %d...", choose);
			buf_set_len(button_ids, choose);
			buf_zero(button_ids);
			if (permute(m, buf_len(m.buttons), choose, 0, button_ids, is_valid_joltage)) {
				result += choose;
				found_valid = true;
				break;
			}
		}
		printf("\n");
		if (!found_valid) {
			printf("failed to find initialization for machine[%d]\n", machine_i);
		}
	}

	printf("part two: %d\n", result);
}

void draw_light_frame(int mid, bool *frame, int num_lights) {
	int num_rows = SCREEN_SIZE / (CELL_SIZE + MACHINE_PAD_Y);
	int mrow = mid % num_rows;
	int mcol = mid / num_rows;
	for (int i=0; i<num_lights; ++i) {
		U32 color = frame[i] ? GREEN_ON : GREEN_OFF;
		int y = mrow * (CELL_SIZE + MACHINE_PAD_Y);
		int x = 1 + (mcol * MACHINE_PAD_X) + (i * (CELL_SIZE + CELL_PAD));
		drawbox(x, y, CELL_SIZE, CELL_SIZE, color);
	}
}

void draw_presses(Machine m, int mid) {
	int num_rows = SCREEN_SIZE / (CELL_SIZE + MACHINE_PAD_Y);
	int mrow = mid % num_rows;
	int mcol = mid / num_rows;
	int y = 2 + mrow * (CELL_SIZE + MACHINE_PAD_Y);
	int x = 10 + (mcol * MACHINE_PAD_X) + (10 * (CELL_SIZE + CELL_PAD));
	drawstringf(x, y, 1, 0xffffff, "%d", m.min_presses);
}

void draw_machine(Machine m, int mid) {
	int num_rows = SCREEN_SIZE / (CELL_SIZE + MACHINE_PAD_Y);
	int mrow = mid % num_rows;
	int mcol = mid / num_rows;
	for (int i=0; i<buf_len(m.light_schema); ++i) {
		U32 color = m.lights[i].on ? GREEN_ON : GREEN_OFF;
		int y = mrow * (CELL_SIZE + MACHINE_PAD_Y);
		int x = 1 + (mcol * MACHINE_PAD_X) + (i * (CELL_SIZE + CELL_PAD));
		drawbox(x, y, CELL_SIZE, CELL_SIZE, color);
	}
}

bool permute_draw(Machine *machine, int machine_id, int n, int k, int depth, int *button_ids, ValidationFunc validate) {
	bool valid = validate(*machine, button_ids, depth); // toggle lights
	if (machine->frame++ % machine->skip_frames == 0 || (depth == k && valid)) {
		for (int i=0; i<buf_len(machine->lights); ++i) {
			buf_push(machine->light_frames, machine->lights[i].on);
		}
		if (machine->frame > 100000)      machine->skip_frames = 100000;
		else if (machine->frame > 10000) machine->skip_frames = 10000;
		else if (machine->frame > 1000) machine->skip_frames = 1000;
		else if (machine->frame > 100) machine->skip_frames = 100;
		else if (machine->frame > 50) machine->skip_frames = 10;
	}
	buf_zero(machine->lights);

	if (depth == k) {

	} else {
		valid = false;
		for (int i=0; i<n; ++i) {
			button_ids[depth] = i;
			if (permute_draw(machine, machine_id, n, k, depth+1, button_ids, validate)) {
				valid = true;
				break;
			}
		}
	}
	return valid;
}

void visualize(BUF(Machine *machines)) {
	int result = 0;
	BUF(int *button_ids) = NULL;

	for (int machine_i = 0; machine_i < buf_len(machines); ++machine_i) {
		Machine *m = &machines[machine_i];
		bool found_valid = false;
		for (int choose=1; choose<=10; ++choose) {
			buf_set_len(button_ids, choose);
			buf_zero(button_ids);
			if (permute_draw(m, machine_i, buf_len(m->buttons), choose, 0, button_ids, is_valid_light)) {
				m->min_presses = choose;
				result += choose;
				found_valid = true;
				break;
			}
		}
		if (!found_valid) {
			printf("failed to find initialization for machine[%d]\n", machine_i);
		}
	}

	setupgif(1, 1, "factory.gif");


	int max_frames = 0;
	for (int i = 0; i < buf_len(machines); ++i) {
		int frames = buf_len(machines[i].light_frames) / buf_len(machines[i].lights);
		if (frames > max_frames) max_frames = frames;
	}
	printf("max frames %d\n", max_frames);

	for (int frame = 0; frame < max_frames; ++frame) {
		for (int i = 0; i < buf_len(machines); ++i) {
			int num_lights = buf_len(machines[i].lights);
			int num_frames = buf_len(machines[i].light_frames) / buf_len(machines[i].lights);
			bool *light_frame;
			if (frame >= num_frames || frame == max_frames - 1) {
				light_frame = machines[i].light_frames + num_lights * (num_frames-1);
				draw_presses(machines[i], i);
			} else {
				light_frame = machines[i].light_frames + num_lights * frame;
			}
			draw_light_frame(i, light_frame, num_lights);
		}
		nextframe();
	}

	drawstringf(875, 965, 3, 0xffffff, "%d", result);

	for (int i=0; i<16; ++i) nextframe();
	endgif();
	printf("visualize: %d\n", result);
}


BUF(Machine *) parse_machines(char *file_name, char *file_data) {
	BUF(Machine *machines) = NULL;
	init_lexer(file_name, file_data);

	while (!is_token(TOKEN_EOF)) {
		Machine m = {.skip_frames = 1};

		expect_token('[');
		while (!is_token(']')) {
			Light l = {0};
			switch(_token.kind) {
				case '.': l.on = false; break;
				case '#': l.on = true;  break;
				default: fatal("expected one of . or #, got %c", _token.kind); break;
			}
			buf_push(m.light_schema, l);
			next_token();
		}
		expect_token(']');

		while (match_token('(')) {
			Button button = {0};
			while (is_token(TOKEN_INT)) {
				int i = (int)parse_int();
				buf_push(button.ids, i);
				match_token(',');
			}
			buf_push(m.buttons, button);
			expect_token(')');
		}

		expect_token('{');
		for (int i=0; is_token(TOKEN_INT); ++i) {
			int joltage = (int)parse_int();
			assert(i < buf_len(m.light_schema));
			m.light_schema[i].joltage = joltage;
			match_token(',');
		}
		expect_token('}');

		buf_set_len(m.lights, buf_len(m.light_schema));
		buf_zero(m.lights);

		buf_push(machines, m);

		match_token('\n');
	}

	return machines;
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

	BUF(Machine *machines) = parse_machines(filename, file_data);

	part_one(machines);
	// part_two(machines);
	visualize(machines);

	return 0;
}
