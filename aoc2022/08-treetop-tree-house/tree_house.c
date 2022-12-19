#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define GIF_WIDTH  (1024)
#define GIF_HEIGHT (1024)
#define SKIP_FRAMES 16
#define SPEED 0.1
#define AMPLITUDE 25
#define FRAME_START_GROW   50
#define FRAME_START_SHRINK 100

typedef enum {
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST
} Direction;

typedef struct {
	float x, y, z;
} Vec3f;

typedef struct {
	Vec3f pos;
	Vec3f up;
	Vec3f forward;
	Vec3f target;
} Camera;

typedef struct {
	int row, col;
} Coord;

typedef float Mat4f[4][4];

global U64 frame_count = 0;

global Camera camera = {0};
global Mat4f ortho_projection = {0};
global Mat4f perspective_projection = {0};
global Mat4f view_matrix = {0};

global Coord *part_one_visible = NULL;

Vec3f normalized(Vec3f a) {
	float magnitude = sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
	assert(magnitude != 0);
	return (Vec3f){
		.x = a.x / magnitude,
		.y = a.y / magnitude,
		.z = a.z / magnitude
	};
}

Vec3f cross(Vec3f a, Vec3f b) {
	return (Vec3f){
		.x = a.y*b.z - a.z*b.y,
		.y = a.z*b.x - a.x*b.z,
		.z = a.x*b.y - a.y*b.x
	};
}

Vec3f vec3f_sub(Vec3f a, Vec3f b) {
	return (Vec3f){
		.x = a.x - b.x,
		.y = a.y - b.y,
		.z = a.z - b.z
	};
}

float lerpf(float a, float b, float t) {
	return a + (b-a)*t;
}

float dot(Vec3f a, Vec3f b) {
	return a.x*b.x + a.y+b.y + a.z*b.z;
}

void create_orthographic_matrix(
	float b, float t, float l, float r,
    float n, float f,
	Mat4f M)
{
    M[0][0] = 2 / (r - l);
    M[0][1] = 0;
    M[0][2] = 0;
    M[0][3] = 0;

    M[1][0] = 0;
    M[1][1] = 2 / (t - b);
    M[1][2] = 0;
    M[1][3] = 0;

    M[2][0] = 0;
    M[2][1] = 0;
    M[2][2] = -2 / (f - n);
    M[2][3] = 0;

    M[3][0] = -(r + l) / (r - l);
    M[3][1] = -(t + b) / (t - b);
    M[3][2] = -(f + n) / (f - n);
    M[3][3] = 1;
}

void create_perspective_matrix(float fovy_degrees, float aspect, float near, float far, Mat4f M) {
	float fovy_radians = fovy_degrees * M_PI / 180;
	float f = 1 / tanf(0.5*fovy_radians);

    M[0][0] = f / aspect;
    M[0][1] = 0;
    M[0][2] = 0;
    M[0][3] = 0;

    M[1][0] = 0;
    M[1][1] = f;
    M[1][2] = 0;
    M[1][3] = 0;

    M[2][0] = 0;
    M[2][1] = 0;
    M[2][2] = (far+near) / (near-far);
    M[2][3] = -1;

    M[3][0] = 0;
    M[3][1] = 0;
    M[3][2] = (2*near*far) / (near-far);
    M[3][3] = 0;
}

void calc_view_matrix(Camera *cam, Mat4f view_matrix) {
	Vec3f z_axis = cam->forward;
	Vec3f y_axis = cam->up;
	Vec3f x_axis = normalized(cross(y_axis, z_axis)); 

    // Create a 4x4 orientation matrix from the right, up, and forward vectors
    // This is transposed which is equivalent to performing an inverse
    // if the matrix is orthonormalized (in this case, it is).
	view_matrix[0][0] = x_axis.x;
	view_matrix[0][1] = y_axis.x;
	view_matrix[0][2] = z_axis.x;
	view_matrix[0][3] = 0;

	view_matrix[1][0] = x_axis.y;
	view_matrix[1][1] = y_axis.y;
	view_matrix[1][2] = z_axis.y;
	view_matrix[1][3] = 0;

	view_matrix[2][0] = x_axis.z;
	view_matrix[2][1] = y_axis.z;
	view_matrix[2][2] = z_axis.z;
	view_matrix[2][3] = 0;

	view_matrix[3][0] = -dot(x_axis, cam->pos); 
	view_matrix[3][1] = -dot(y_axis, cam->pos); 
	view_matrix[3][2] = -dot(z_axis, cam->pos);
	view_matrix[3][3] = 1;
}

Vec3f mul_point_matrix(Vec3f p, Mat4f M) {
	Vec3f out;
	//out = in * Mproj;
	out.x   = p.x * M[0][0] + p.y * M[1][0] + p.z * M[2][0] + /* p.z = 1 */ M[3][0];
	out.y   = p.x * M[0][1] + p.y * M[1][1] + p.z * M[2][1] + /* p.z = 1 */ M[3][1];
	out.z   = p.x * M[0][2] + p.y * M[1][2] + p.z * M[2][2] + /* p.z = 1 */ M[3][2];
	float w = p.x * M[0][3] + p.y * M[1][3] + p.z * M[2][3] + /* p.z = 1 */ M[3][3];

	// normalize if w is different than 1 (convert from homogeneous to Cartesian coordinates)
	if (w != 1) {
		out.x /= w;
		out.y /= w;
		out.z /= w;
	}
	return out;
}

void camera_init(void) {
	float b, t, l, r, n, f, max;

	camera.pos = (Vec3f){600, 600, 600};
	camera.up = (Vec3f){0, 1, 0};
	camera.forward = (Vec3f){0, 0, 1};

	camera.target = (Vec3f){490, 100, 490};

	max = 1024;
	r = max * (GIF_WIDTH / (float)GIF_HEIGHT);
	t = max; l = -r; b = -t;
	n = 0.1; f = 100;

	create_orthographic_matrix(b, t, l, r, n, f, ortho_projection);
	create_perspective_matrix(60, GIF_WIDTH/GIF_HEIGHT, 0.1, 100, perspective_projection);
}

void camera_update(Camera *cam, int frame_count) {
	cam->pos.x = 980 * sin(frame_count*0.05);
	cam->pos.z = 980 * cos(frame_count*0.05);

	// set the forward and up vectors based on target
	Vec3f up = {0, 1, 0};
	cam->forward = normalized(vec3f_sub(cam->pos, cam->target));
    Vec3f right = normalized(cross(up, cam->forward));
    cam->up = cross(cam->forward, right);

	calc_view_matrix(cam, view_matrix);
}

bool visible_from_direction(int *grid, int rows, int cols, int row, int col, Direction direction) {
	int i, j, w = cols;
	int height = grid[row*cols+col];

	bool visible = true;

	switch (direction) {
	case DIRECTION_NORTH:
		i = col;
		for (j=row-1; j>=0; --j)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	case DIRECTION_EAST:
		j = row;
		for (i=col+1; i<cols; ++i)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	case DIRECTION_SOUTH:
		i = col;
		for (j=row+1; j<rows; ++j)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	case DIRECTION_WEST:
		j = row;
		for (i=col-1; i>=0; --i)
			if (grid[j*w+i] >= height) 
				visible = false;
		break;
	default:
		fprintf(stderr, "visible_from_direction: unknown direction %d\n", direction);
		exit(1);
		break;
	}

	return visible;
}

int view_distance_in_direction(int *grid, int rows, int cols, int row, int col, Direction direction) {
	int i, j, w = cols;
	int height = grid[row*cols+col];

	int distance = 0;

	switch (direction) {
	case DIRECTION_NORTH:
		i = col;
		for (j=row-1; j>=0; --j) {
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	case DIRECTION_EAST:
		j = row;
		for (i=col+1; i<cols; ++i) {
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	case DIRECTION_SOUTH:
		i = col;
		for (j=row+1; j<rows; ++j) {
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	case DIRECTION_WEST:
		j = row;
		for (i=col-1; i>=0; --i) { 
			++distance;
			if (grid[j*w+i] >= height) 
				return distance;
		}
		break;
	default:
		fprintf(stderr, "visible_from_direction: unknown direction %d\n", direction);
		exit(1);
		break;
	}

	return distance;
}

void draw_test(void) {
	int sep = 10;
	int i, j=0, k=0;
	Vec3f p;
	U32 color;
	for (i=0; i<20; ++i)
	for (j=0; j<20; ++j)
	for (k=0; k<20; ++k)
	{
		p = (Vec3f){i*sep, j*sep, k*sep};

		p = mul_point_matrix(p, view_matrix);
		p = mul_point_matrix(p, perspective_projection);

		/*p = mul_point_matrix(p, ortho_projection);*/

		if (i == 0)       color = 0x0000FF;
		else if (j == 0)  color = 0x00FF00;
		else if (k == 0)  color = 0xFF0000;
		else color = 0xFFFFFF;

		/*if (j < 5)       color = 0x0000FF;*/
		/*else if (j < 10) color = 0x00FFFF;*/
		/*else if (j < 15) color = 0x00FF00;*/
		/*else             color = 0xFF0000;*/

		/*printf("point at (%f,%f)\n", p.x, p.y);*/

		float x = (p.x * 0.5f + 1) * GIF_WIDTH;
		float y = (p.y * 0.5f + 1) * GIF_HEIGHT;

		drawcircle(x, y, 5, color);
	}
}

void draw_tree(int row, int col, int treetop_x, int treetop_y) {
	int margin = 10;

	Vec3f p0 = {
		.x = col*margin,
		.y = 0,
		.z = row*margin,
	};

	p0 = mul_point_matrix(p0, view_matrix);
	p0 = mul_point_matrix(p0, perspective_projection);

	int x0 = (p0.x  + 1) * 0.5 * GIF_WIDTH + 400;
	int y0 = GIF_HEIGHT - (p0.y  + 1) * 0.5 * GIF_HEIGHT;

	if (frame_count < FRAME_START_GROW)
		drawcircle(x0, y0, 2, 0x00BB00);
	else if (treetop_y != y0)
		drawline(x0, y0, treetop_x, treetop_y, 2, 0x00BB00);
	else
		drawcircle(x0, y0, 2, 0x00BB00);
}

void draw_forest(int *grid, int rows, int cols) {
	(void)rows;

	int i, count = arrlen(part_one_visible);

	assert(count < 2048);
	local_persist float t[2048] = {0}; // t values for lerping height in draw_tree

	for (i=0; i<count; ++i) {
		int margin = 10;
		Coord coord = part_one_visible[i];
		float height = grid[coord.row * cols + coord.col];
		int scale_y = 50;
		float render_height = lerpf(0, height*scale_y, t[i]);
		Vec3f p1 = {
			.x = coord.col*margin,
			.y = render_height,
			.z = coord.row*margin,
		};
		p1 = mul_point_matrix(p1, view_matrix);
		p1 = mul_point_matrix(p1, perspective_projection);
		int x1 = (p1.x  + 1) * 0.5 * GIF_WIDTH + 400;
		int y1 = GIF_HEIGHT - (p1.y  + 1) * 0.5 * GIF_HEIGHT;

		if (frame_count >= FRAME_START_SHRINK) {
			t[i] -= 0.05;
			if (t[i] < 0) t[i] = 0;
		} else if (frame_count >= FRAME_START_GROW) {
			t[i] += 0.05;
			if (t[i] > 1) t[i] = 1;
		} 
		draw_tree(coord.row, coord.col, x1, y1);
	}
}

void part_one(int *grid, int rows, int cols) {
	int i, j, visible = 0;
	Coord coord = {0};
	for (j=1; j<rows-1; ++j) {
		for (i=1; i<cols-1; ++i) {
			if (
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_NORTH) ||
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_EAST) ||
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_SOUTH) ||
				visible_from_direction(grid, rows, cols, j, i, DIRECTION_WEST)
			){
				++visible;

				coord.row = j;
				coord.col = i;
				arrput(part_one_visible, coord);
			}
		}
	}

	visible += 2*cols + 2*(rows-2);

	printf("part_one: %d\n", visible);
}

void part_two(int *grid, int rows, int cols) {
	int i, j, score = 0, best_score = 0;
	for (j=0; j<rows; ++j) {
		for (i=0; i<cols; ++i) {
			score =
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_NORTH) *
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_EAST)  *
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_SOUTH) * 
				view_distance_in_direction(grid, rows, cols, j, i, DIRECTION_WEST);
			best_score = Max(score, best_score);
		}
	}
	printf("part two: %d\n", best_score);
}

int *build_grid(U8 *input, int *out_rows, int *out_cols) {
	int *grid = NULL;
	char *line;
	int cols, rows = 0;
	int height;
	do {
		cols = 0;
		// parse each character in line as int and put in grid
		for (line = arb_chop_by_delimiter((char **)&input, "\n"); *line != '\0'; ++line) {
			height = *line - 48;
			arrput(grid, height);
			++cols;
		}
		*out_cols = cols;
		++rows;
	} while (*input != '\0');

	nextframe();

	*out_rows = rows;
	return grid;
}


int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "Must supply filepath to puzzle input\n");
		exit(1);
	}

	FILE *fp;
	U8 *file_data;
	size_t file_size;
	int rows, cols, *grid;

	fp = fopen(argv[1], "r");
	if (!fp) { perror("fopen"); exit(1); }

	int ok = arb_read_entire_file(fp, &file_data, &file_size);
	if (ok != ARB_READ_ENTIRE_FILE_OK) {
		fprintf(stderr, "Error: arb_read_entire_file()\n");
		fclose(fp);
		exit(1);
	}

	fclose(fp);

	setupgif(0, 2, "tree_house.gif");

	grid = build_grid(file_data, &rows, &cols);

	part_one(grid, rows, cols);
	part_two(grid, rows, cols);

	camera_init();
	for (frame_count = 0; frame_count < 250; ++frame_count) {
		camera_update(&camera, frame_count);
		clear();
		draw_forest(grid, rows, cols);
		/*draw_test();*/
		nextframe();
	}

	endgif();

	return 0;
}
