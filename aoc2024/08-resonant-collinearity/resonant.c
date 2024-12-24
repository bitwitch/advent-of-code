#include "../common.c"

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	char kind;
	Vec2 pos;
} Antenna;

typedef struct {
	BUF(Antenna *antennas);
	BUF(Antenna *antinodes);
	int size;
} Field;

typedef bool (*CmpFunc)(Vec2, Vec2);

Vec2 vec2_sub(Vec2 a, Vec2 b) {
	return (Vec2){a.x - b.x, a.y - b.y};
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
	return (Vec2){a.x + b.x, a.y + b.y};
}

void *vec2_hash(Vec2 v) {
	char buf[64] = {0};
	snprintf(buf, ARRAY_COUNT(buf), "x%dy%d", v.x, v.y);
	char *hash = str_intern(buf);
	return (void*)hash;
}

int gcd(int a, int b) {
	a = abs(a);
	b = abs(b);
	if (a == 0) return b;
	if (b == 0) return a;
	if (a == b) return a;
	if (a > b)  return gcd(a - b, b);
	return gcd(a, b - a);
}

bool every(Vec2 a, Vec2 b) {
	(void)a; (void)b;
	return true;
}

bool one_twice_as_far_as_other(Vec2 a, Vec2 b) {
	a.x = abs(a.x);
	a.y = abs(a.y);
	b.x = abs(b.x);
	b.y = abs(b.y);
	if (a.x < b.x) {
		return b.x == 2*a.x && b.y == 2*a.y;
	} else {
		return a.x == 2*b.x && a.y == 2*b.y;
	}	
}


bool is_on_field(Field *field, Vec2 pos) {
	return pos.x >= 0 && pos.x < field->size &&
		   pos.y >= 0 && pos.y < field->size;
}


void find_antinodes(Field *field, Map *unique, Antenna a, Antenna other, 
		            Vec2 slope, CmpFunc cmp_func) 
{
	Vec2 step = vec2_add(a.pos, slope);
	while (is_on_field(field, step)) {
		Vec2 dist_a     = vec2_sub(step, a.pos);
		Vec2 dist_other = vec2_sub(step, other.pos);
		if (cmp_func(dist_a, dist_other)) {
			void *key = vec2_hash(step);
			if (map_get(unique, key) == NULL) {
				buf_push(field->antinodes, (Antenna){
					.kind = a.kind,
					.pos = step,
				});
				map_put(unique, key, (void*)1);
			}
		}
		step = vec2_add(step, slope);
	}
}

void part_one(Field *field) {
	Map unique = {0};
	for (int j=0; j<buf_len(field->antennas); ++j) {
		Antenna a = field->antennas[j];
		for (int i=0; i<buf_len(field->antennas); ++i) {
			if (i == j) continue;
			Antenna other = field->antennas[i];
			if (a.kind == other.kind) {
				Vec2 slope = vec2_sub(other.pos, a.pos);
				int divisor = gcd(slope.x, slope.y);
				slope.x /= divisor;
				slope.y /= divisor;
				Vec2 neg_slope = (Vec2){-slope.x, -slope.y};
				find_antinodes(field, &unique, a, other, slope,
					one_twice_as_far_as_other);
				find_antinodes(field, &unique, a, other, neg_slope,
					one_twice_as_far_as_other);
			}
		}
	}
	printf("part one: %d\n", buf_len(field->antinodes));
}

void part_two(Field *field) {
	Map unique = {0};
	for (int j=0; j<buf_len(field->antennas); ++j) {
		Antenna a = field->antennas[j];
		for (int i=0; i<buf_len(field->antennas); ++i) {
			if (i == j) continue;
			Antenna other = field->antennas[i];
			if (a.kind == other.kind) {
				Vec2 slope = vec2_sub(other.pos, a.pos);
				int divisor = gcd(slope.x, slope.y);
				slope.x /= divisor;
				slope.y /= divisor;
				Vec2 neg_slope = (Vec2){-slope.x, -slope.y};
				find_antinodes(field, &unique, a, other, slope, every);
				find_antinodes(field, &unique, a, other, neg_slope, every);
			}
		}
	}
	printf("part two: %d\n", buf_len(field->antinodes));
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

	Field field = {0};

	int x = 0, y = 0;
	for (char *c = file_data; *c != 0; ++c) {
		if (isalnum(*c)) {
			Antenna a = {0};
			a.kind = *c;
			a.pos = (Vec2){x,y};
			buf_push(field.antennas, a);
		} else if (*c == '\n') {
			++y;
			x = -1;
		} 
		++x;
	}

	field.size = y;

	part_one(&field);
	buf_set_len(field.antinodes, 0);
	part_two(&field);

	return 0;
}

