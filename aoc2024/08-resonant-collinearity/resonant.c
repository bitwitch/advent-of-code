#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_WIDTH  1024 
#define SCREEN_HEIGHT 1024 
#define LINE_W        2

#define ANTENNA_COLOR  0xFFBDBDBD
#define WHITE          0xFFF7F5EA
#define RED            0xFF28289A
#define BRIGHT_RED     0xFF4242EC
#define GREEN          0xFF34A82E
#define GOLD           0xFF15B4EC

typedef struct {
	int x, y;
} Vec2;

typedef struct {
	char kind;
	Vec2 pos;
	U32 color;
} Antenna;

typedef struct {
	char kind;
	bool active;
	Vec2 pos;
	Antenna *antenna1;
	Antenna *antenna2;
	Antenna *overlap_antenna;
} Antinode;

typedef struct {
	BUF(Antenna *antennas);
	BUF(Antinode *antinodes);
	int size;
} Field;

typedef struct {
	char kind;
	Vec2 p0, p1;
	Vec2 slope;
	bool active;
	Antenna *antenna;
} Line;

typedef bool (*CmpFunc)(Vec2, Vec2);

int cell_size = 0; 
int field_pad = 0;

U32 colors[] = { 0xff0e0eaf, 0xff0e1daf, 0xff0e2caf, 0xff0e3baf, 0xff0e4baf, 0xff0e5aaf, 0xff0e69af, 0xff0e78af, 0xff0e87af, 0xff0e97af, 0xff0ea6af, 0xff0eafaa, 0xff0eaf9b, 0xff0eaf8b, 0xff0eaf7c, 0xff0eaf6d, 0xff0eaf5e, 0xff0eaf4e, 0xff0eaf3f, 0xff0eaf30, 0xff0eaf21, 0xff0eaf11, 0xff19af0e, 0xff28af0e, 0xff37af0e, 0xff47af0e, 0xff56af0e, 0xff65af0e, 0xff74af0e, 0xff84af0e, 0xff93af0e, 0xffa2af0e, 0xffafae0e, 0xffaf9e0e, 0xffaf8f0e, 0xffaf800e, 0xffaf710e, 0xffaf610e, 0xffaf520e, 0xffaf430e, 0xffaf340e, 0xffaf240e, 0xffaf150e, 0xffaf0e15, 0xffaf0e24, 0xffaf0e34, 0xffaf0e43, 0xffaf0e52, 0xffaf0e61, 0xffaf0e71, 0xffaf0e80, 0xffaf0e8f, 0xffaf0e9e, 0xffaf0eae, 0xffa20eaf, 0xff930eaf, 0xff840eaf, 0xff740eaf, 0xff650eaf, 0xff560eaf };

Vec2 vec2_sub(Vec2 a, Vec2 b) {
	return (Vec2){a.x - b.x, a.y - b.y};
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
	return (Vec2){a.x + b.x, a.y + b.y};
}

Vec2 vec2_mul(Vec2 a, Vec2 b) {
	return (Vec2){a.x * b.x, a.y * b.y};
}

bool vec2_equal(Vec2 a, Vec2 b) {
	return a.x == b.x && a.y == b.y;
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


bool is_in_bounds(int size, Vec2 point) {
	return point.x >= 0 && point.x < size &&
		   point.y >= 0 && point.y < size;
}

void find_antinodes(Field *field, Map *unique, Antenna *a, Antenna *other,
		            Vec2 slope, CmpFunc cmp_func) 
{
	Vec2 step = vec2_add(a->pos, slope);
	while (is_in_bounds(field->size, step)) {
		Vec2 dist_a     = vec2_sub(step, a->pos);
		Vec2 dist_other = vec2_sub(step, other->pos);
		if (cmp_func(dist_a, dist_other)) {
			void *key = vec2_hash(step);
			if (map_get(unique, key) == NULL) {
				Antenna *overlap_antenna = NULL;
				for (int i=0; i<buf_len(field->antennas); ++i) {
					if (vec2_equal(field->antennas[i].pos, step)) {
						overlap_antenna = &field->antennas[i];
					}
				}
				buf_push(field->antinodes, (Antinode){
					.kind = a->kind,
					.pos = step,
					.antenna1 = a,
					.antenna2 = other,
					.overlap_antenna = overlap_antenna,
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
		Antenna *a = &field->antennas[j];
		for (int i=0; i<buf_len(field->antennas); ++i) {
			if (i == j) continue;
			Antenna *other = &field->antennas[i];
			if (a->kind == other->kind) {
				Vec2 slope = vec2_sub(other->pos, a->pos);
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
		Antenna *a = &field->antennas[j];
		for (int i=0; i<buf_len(field->antennas); ++i) {
			if (i == j) continue;
			Antenna *other = &field->antennas[i];
			if (a->kind == other->kind) {
				Vec2 slope = vec2_sub(other->pos, a->pos);
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

void draw_antenna(Antenna a) {
	// TODO(shaw): diff color for diff antenna kinds
	
	int x = a.pos.x * cell_size + field_pad;
	int y = a.pos.y * cell_size + field_pad;

	// left leg
	int x0 = x;
	int y0 = y + cell_size;
	int x1 = x + cell_size/2;
	int y1 = y;
	drawline(x0, y0, x1, y1, 2, ANTENNA_COLOR);

	// middle leg
	// x0 = x + cell_size/2;
	// y0 = y;
	// y0 = y1;
	// x1 = x0;
	// y1 = y + cell_size - 2;
	// drawline(x0, y0, x1, y1, 2, ANTENNA_COLOR);

	// right leg
	x0 = x + cell_size/2;
	y0 = y;
	x1 = x + cell_size;
	y1 = y + cell_size;
	drawline(x0, y0, x1, y1, 2, ANTENNA_COLOR);

	// antenna topper
	drawcircle(x0, y0, 5, a.color);

	// left support bar
	// x0 = x + cell_size/4 + 2;
	// y0 = y + cell_size/2 - 8;
	// x1 = x + cell_size/2;
	// y1 = y + cell_size/2 + 8;
	// drawline(x0, y0, x1, y1, 2, ANTENNA_COLOR);

	// right support bar
	// x0 = x1;
	// y0 = y1;
	// x1 = x + (3*cell_size/4) - 2;
	// y1 = y + cell_size/2 - 8;
	// drawline(x0, y0, x1, y1, 2, ANTENNA_COLOR);
}

int clamp(int x, int min, int max) {
	x = MAX(min, x);
	return MIN(max, x);
}

void draw_field(Field *field, BUF(Line *lines)) {
	for (int i=0; i<buf_len(field->antinodes); ++i) {
		Antinode an = field->antinodes[i];
		if (an.active) {
			int x = an.pos.x * cell_size + field_pad;
			int y = an.pos.y * cell_size + field_pad;
			drawbox(x, y, cell_size, cell_size, RED);
		}
	}
	for (int i=0; i<buf_len(lines); ++i) {
		Line line = lines[i];
		if (line.active && !vec2_equal(line.p0, line.p1)) {
			int x0 = line.p0.x + field_pad;
			int y0 = line.p0.y + field_pad;
			int x1 = line.p1.x + field_pad;
			int y1 = line.p1.y + field_pad;

			// int x0 = clamp(line.p0.x + field_pad, 0, SCREEN_WIDTH);
			// int y0 = clamp(line.p0.y + field_pad, 0, SCREEN_HEIGHT);
			// int x1 = clamp(line.p1.x + field_pad, 0, SCREEN_WIDTH);
			// int y1 = clamp(line.p1.y + field_pad, 0, SCREEN_HEIGHT);

			drawline(x0, y0, x1, y1, LINE_W, BRIGHT_RED);
		}
	}
	for (int i=0; i<buf_len(field->antennas); ++i) {
		draw_antenna(field->antennas[i]);
	}
}

bool all_lines_extended(BUF(Line *lines)) {
	for (int i=0; i<buf_len(lines); ++i) {
		if (is_in_bounds(SCREEN_WIDTH, lines[i].p0) ||
		    is_in_bounds(SCREEN_WIDTH, lines[i].p1))
		{
			return false;
		}
	}
	return true;
}

void activate_antinodes(Field *field, Line *line) {
	for (int i=0; i<buf_len(field->antinodes); ++i) {
		Antinode *an = &field->antinodes[i];

		if (line->antenna != an->overlap_antenna &&
		   (an->antenna1 == line->antenna || an->antenna2 == line->antenna) &&
			an->kind == line->kind && 
			!an->active)
		{
			int x = an->pos.x * cell_size;
			int y = an->pos.y * cell_size;
			
			for (Vec2 p = line->p0;
				 !vec2_equal(p, line->p1);
				 p = vec2_add(p, line->slope))
			{
				if (p.x >= x && p.x <= x + cell_size &&
					p.y >= y && p.y <= y + cell_size)
				{
					an->active = true;
				}
			}

			// if (line->p0.x >= x && line->p0.x <= x + cell_size &&
				// line->p0.y >= y && line->p0.y <= y + cell_size)
			// {
				// an->active = true;
			// }
			// if (line->p1.x >= x && line->p1.x <= x + cell_size &&
				// line->p1.y >= y && line->p1.y <= y + cell_size)
			// {
				// an->active = true;
			// }
		}
	}
}

void visualize(Field *field) {
	setupgif(0, 1, "resonant.gif");

	cell_size = SCREEN_WIDTH / field->size;
	field_pad = (SCREEN_WIDTH % field->size) / 2;

	BUF(Line *lines) = NULL;
	Map unique = {0};
	Map antenna_colors = {0};
	int color_index = 0;
	for (int j=0; j<buf_len(field->antennas); ++j) {
		Antenna *a = &field->antennas[j];

		if (map_get(&antenna_colors, (void*)a->kind) == NULL) {
			assert(color_index < (int)(ARRAY_COUNT(colors)));
			map_put(&antenna_colors, (void*)a->kind, (void*)colors[color_index++]);
		}
		a->color = (U32)map_get(&antenna_colors, (void*)a->kind);

		for (int i=j; i<buf_len(field->antennas); ++i) {
			if (i == j) continue;
			Antenna *other = &field->antennas[i];
			if (a->kind == other->kind) {
				Vec2 slope = vec2_sub(other->pos, a->pos);
				int divisor = gcd(slope.x, slope.y);
				slope.x /= divisor;
				slope.y /= divisor;
				Vec2 neg_slope = (Vec2){-slope.x, -slope.y};
				find_antinodes(field, &unique, a, other, slope, every);
				find_antinodes(field, &unique, a, other, neg_slope, every);

				Line line = {0};
				line.antenna = &field->antennas[j];
				Vec2 cell_dims = {cell_size, cell_size};
				Vec2 origin = vec2_mul(a->pos, cell_dims);
				origin.x += cell_size/2;
				origin.y += cell_size/2;
				line.kind = a->kind;
				line.slope = slope;
				line.p0 = origin;
				line.p1 = origin;
				buf_push(lines, line);
			}
		}
	}

	int frames = 0;
	int skip_frames = 2;
	int next_line = 0;
	int next_line_cooldown = 10;
	int next_line_counter = 0;

	while (!all_lines_extended(lines)) {
		clear();
		draw_field(field, lines);
		if ((frames++ % skip_frames) == 0)
			nextframe();

		// accelerate animation
		if ((frames % 100) == 0) {
			++skip_frames;
		}

		// accelerate activating new lines
		if ((frames % 25) == 0) {
			if (next_line_cooldown > 0) {
				--next_line_cooldown;
			}
		}

		// activate a new line
		if (next_line_counter <= 0 && next_line < buf_len(lines)) {
			lines[next_line++].active = true;
			next_line_counter = next_line_cooldown;
		} else {
			--next_line_counter;
		}

		// extend active lines
		for (int i=0; i<buf_len(lines); ++i) {
			if (lines[i].active) {
				Vec2 p0 = lines[i].p0;
				Vec2 p1 = lines[i].p1;
				bool p0_in_bounds = false;
				bool p1_in_bounds = false;
				if (is_in_bounds(SCREEN_WIDTH, p0)) {
					lines[i].p0 = vec2_sub(p0, lines[i].slope);
					p0_in_bounds = true;
				}
				if (is_in_bounds(SCREEN_WIDTH, p1)) {
					lines[i].p1 = vec2_add(p1, lines[i].slope);
					p1_in_bounds = true;
				}
				if (!p0_in_bounds && !p1_in_bounds) {
					lines[i].active = false;
				}
				activate_antinodes(field, &lines[i]);
			}
		}
	}

	for (int i=0; i<buf_len(lines); ++i) {
		lines[i].active = false;
	}
	clear();
	draw_field(field, lines);
	for (int i=0; i<20; ++i) {
		nextframe();
	}

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
	buf_set_len(field.antinodes, 0);

	visualize(&field);

	return 0;
}

