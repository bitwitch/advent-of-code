#include "../common.c"
#include "../lex.c"

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

bool vec2_equal(Vec2 a, Vec2 b) {
	return a.x == b.x && a.y == b.y;
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
	Loc guard_start = map->guard;
	while (is_on_map(map, map->guard.pos)) {
		char buf[32] = {0};
		snprintf(buf, ARRAY_COUNT(buf), "x%dy%d", 
			map->guard.pos.x, map->guard.pos.y);
		char *key = str_intern(buf);
		if (map_get(&visited_map, key) == NULL) {
			pos_count += 1;
			map_put(&visited_map, key, (void*)1);
		}

		Vec2 target = loc_one_step_forward(map->guard);
		if (obstacle_at(map, target)) {
			map->guard.dir = dir_turn_right(map->guard.dir);
		} else {
			map->guard.pos = target;
		}
	}
	map->guard = guard_start;
	printf("part one: %d\n", pos_count);
}

Loc *get_gaurd_path(MapInfo *map) {
	BUF(Loc *path) = NULL;
	Loc guard_start = map->guard;
	while (is_on_map(map, map->guard.pos)) {
		buf_push(path, map->guard);
		Vec2 target = loc_one_step_forward(map->guard);
		if (obstacle_at(map, target)) {
			map->guard.dir = dir_turn_right(map->guard.dir);
		} else {
			map->guard.pos = target;
		}
	}
	map->guard = guard_start;
	return path;
}

bool loc_equal(Loc a, Loc b) {
	return vec2_equal(a.pos, b.pos) && a.dir == b.dir;
}

void part_two(MapInfo *map) {
	int count = 0;
	BUF(Loc *path) = get_gaurd_path(map);

	for (int i=1; i < buf_len(path); ++i) {
		Vec2 obstruction = path[i].pos;
		Loc entry_loc = path[i-1];
		Loc loc = entry_loc;
		loc.dir = dir_turn_right(loc.dir);
		int dir_count = 0;

		while (is_on_map(map, loc.pos) && !loc_equal(loc, entry_loc)) {
			Vec2 target = loc_one_step_forward(loc);
			if (obstacle_at(map, target)) {
				loc.dir = dir_turn_right(loc.dir);
				++dir_count;
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

	return 0;
}

