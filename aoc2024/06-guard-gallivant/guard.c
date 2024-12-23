#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_HEIGHT 1024
#define SCREEN_WIDTH  1024

#define BLACK      0xFF000000
#define GREEN      0xFF00FF00
#define DARK_GREEN 0xFF003300
#define RED        0xFF0000FF

typedef struct {
	int x, y;
} Vec2;

typedef enum {
	UP, 
	RIGHT, 
	DOWN, 
	LEFT
} Direction;

typedef struct {
	Vec2 pos;
	Direction dir;
} Loc;

typedef struct {
	Vec2 bounds;
	Loc guard;
	BUF(Vec2 *obstacles);
	BUF(char **grid);
} MapInfo;

typedef struct {
	BUF(U32 **grid);
	Loc guard;
	int cell_size;
	int offset;
} Display;

bool vec2_equal(Vec2 a, Vec2 b) {
	return a.x == b.x && a.y == b.y;
}

bool loc_equal(Loc a, Loc b) {
	return vec2_equal(a.pos, b.pos) && a.dir == b.dir;
}

Vec2 loc_one_step_forward(Loc loc) {
	Vec2 result = loc.pos;
	switch(loc.dir) {
	case UP:    result.y -= 1; break;
	case RIGHT: result.x += 1; break;
	case DOWN:  result.y += 1; break;
	case LEFT:  result.x -= 1; break;
	default: assert(0); break;
	}
	return result;
}

bool obstacle_at(MapInfo *map, Vec2 target) {
	for (int i=0; i<buf_len(map->obstacles); ++i) {
		if (vec2_equal(target, map->obstacles[i])) {
			return true;
		}
	}
	return false;
}

bool is_on_map(MapInfo *map, Vec2 pos) {
	return pos.x >= 0 && pos.x <= map->bounds.x &&
		   pos.y >= 0 && pos.y <= map->bounds.y;
}

Direction dir_turn_right(Direction dir) {
	return (dir == LEFT) ? UP : dir + 1;
}

void part_one(MapInfo *map) {
	int pos_count = 0;
	Map visited_map = {0};
	Loc loc = map->guard;
	while (is_on_map(map, loc.pos)) {
		char buf[32] = {0};
		snprintf(buf, ARRAY_COUNT(buf), "x%dy%d", loc.pos.x, loc.pos.y);
		char *key = str_intern(buf);
		if (map_get(&visited_map, key) == NULL) {
			pos_count += 1;
			map_put(&visited_map, key, (void*)1);
		}

		Vec2 target = loc_one_step_forward(loc);
		if (obstacle_at(map, target)) {
			loc.dir = dir_turn_right(loc.dir);
		} else {
			loc.pos = target;
		}
	}
	printf("part one: %d\n", pos_count);
}

Loc *get_guard_path(MapInfo *map) {
	BUF(Loc *path) = NULL;
	Loc loc = map->guard;
	while (is_on_map(map, loc.pos)) {
		buf_push(path, loc);
		Vec2 target = loc_one_step_forward(loc);
		if (obstacle_at(map, target)) {
			loc.dir = dir_turn_right(loc.dir);
		} else {
			loc.pos = target;
		}
	}
	return path;
}

Vec2 *get_unique_guard_positions(MapInfo *map) {
	BUF(Vec2 *positions) = NULL;
	Loc loc = map->guard;
	Map visited_map = {0};
	while (is_on_map(map, loc.pos)) {
		char buf[32] = {0};
		snprintf(buf, ARRAY_COUNT(buf), "x%dy%d", loc.pos.x, loc.pos.y);
		char *key = str_intern(buf);
		if (map_get(&visited_map, key) == NULL) {
			map_put(&visited_map, key, (void*)1);
			buf_push(positions, loc.pos);
		}

		Vec2 target = loc_one_step_forward(loc);
		if (obstacle_at(map, target)) {
			loc.dir = dir_turn_right(loc.dir);
		} else {
			loc.pos = target;
		}
	}
	return positions;
}


void part_two_too_slow(MapInfo *map) {
	int count = 0;
	BUF(Loc *path) = get_guard_path(map);

	for (int i=1; i < buf_len(path); ++i) {
		Vec2 obstruction = path[i].pos;
		Loc entry_loc = path[i-1];
		if (vec2_equal(obstruction, entry_loc.pos)) {
			continue;
		}
		Loc loc = entry_loc;
		loc.dir = dir_turn_right(loc.dir);

		while (is_on_map(map, loc.pos) && !loc_equal(loc, entry_loc)) {
			Vec2 target = loc_one_step_forward(loc);
			if (obstacle_at(map, target)) {
				loc.dir = dir_turn_right(loc.dir);
			} else {
				loc.pos = target;
			}
		}
		if (loc_equal(loc, entry_loc)) {
			count += 1;
		}
	}

	printf("part two: %d\n", count);
}

void part_two(MapInfo *map) {
	int count = 0;
	BUF(Vec2 *positions) = get_unique_guard_positions(map);

	for (int i=1; i < buf_len(positions); ++i) {
		Loc loc = map->guard;
		Vec2 obstruction = positions[i];
		Map visited = {0};
		buf_push(map->obstacles, obstruction);
		while (is_on_map(map, loc.pos)) {
			char buf[64] = {0};
			snprintf(buf, ARRAY_COUNT(buf), "x%dy%dd%d",
				loc.pos.x, loc.pos.y, (int)loc.dir);
			char *key = str_intern(buf);
			if (map_get(&visited, key) != NULL) {
				// this means we detected a cycle
				++count;
				break;
			}
			map_put(&visited, key, (void*)1);

			Vec2 target = loc_one_step_forward(loc);
			if (obstacle_at(map, target)) {
				loc.dir = dir_turn_right(loc.dir);
			} else {
				loc.pos = target;
			}
		}
		map_clear(&visited);
		buf_set_len(map->obstacles, buf_len(map->obstacles) - 1);
	}

	printf("part two: %d\n", count);
}

void draw_guard(Display display) {
	int guard_x = display.guard.pos.x * display.cell_size + display.offset;
	int guard_y = display.guard.pos.y * display.cell_size + display.offset;
	drawbox(guard_x, guard_y, display.cell_size, display.cell_size, GREEN);

	// double arrow_pad = 4;
	// double half_cell = 0.5 * display.cell_size;
	// int x0, y0, x1, y1;
	// switch(display.guard.dir) {
		// case UP:
			// x0 = guard_x + half_cell;
			// y0 = guard_y + display.cell_size - arrow_pad;
			// x1 = x0;
			// y1 = guard_y + arrow_pad;
			// drawarrow(x0, y0, x1, y1, 3, BLACK);
			// break;
	// }
}

void draw_map(Display display) {
	// draw static stuff
	for (int j=0; j<buf_len(display.grid); ++j) {
		for (int i=0; i<buf_len(display.grid[0]); ++i) {
			int x = i * display.cell_size + display.offset;
			int y = j * display.cell_size + display.offset;
			U32 color = display.grid[j][i];
			drawbox(x, y, display.cell_size, display.cell_size, color);
		}
	}

	// draw guard
	draw_guard(display);
}

Display init_display(MapInfo *map) {
	Display display = {0};
	for (int j=0; j <= map->bounds.y; ++j) {
		BUF(U32 *row) = NULL;
		for (int i=0; i <= map->bounds.x; ++i) {
			buf_push(row, 0);
		}
		buf_push(display.grid, row);
	}
	for (int i=0; i < buf_len(map->obstacles); ++i) {
		int x = map->obstacles[i].x;
		int y = map->obstacles[i].y;
		display.grid[y][x] = RED;
	}
	display.cell_size = SCREEN_HEIGHT / buf_len(display.grid);
	display.offset = ((int)SCREEN_HEIGHT - (display.cell_size * buf_len(display.grid))) / 2;
	display.guard = map->guard;
	return display;
}

void visualize(MapInfo *map) {
	Display display = init_display(map);
	setupgif(0, 1, "guard.gif");

	U64 skip_frames = 15;
	U64 frames = 0;
	while (is_on_map(map, display.guard.pos)) {
		clear();
		draw_map(display);
		if ((frames++ % skip_frames) == 0)
			nextframe();

		Vec2 target = loc_one_step_forward(display.guard);
		if (obstacle_at(map, target)) {
			display.guard.dir = dir_turn_right(display.guard.dir);
		} else {
			display.guard.pos = target;
			display.grid[target.y][target.x] = DARK_GREEN;
		}
	}

	for (int i=0; i<25; ++i) nextframe();

	endgif();
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

	MapInfo map = {0};
	int x = 0, y = 0;
	for (char *c = file_data; *c != 0; ++c) { 
		if (*c == '\n') {
			++y;
			map.bounds.x = x - 1;
			x = -1;
		} else if (*c == '#') {
			buf_push(map.obstacles, (Vec2){x, y});
		} else if (*c == '^') {
			map.guard.pos.x = x;
			map.guard.pos.y = y;
			map.guard.dir = UP;
		}
		++x;
	}
	map.bounds.y = y - 1;

	part_one(&map);
	part_two(&map);
	visualize(&map);

	return 0;
}

