#include "../common.c"
#include "../../base/base_inc.h"
#include "../../base/base_inc.c"

#define SCREEN_SIZE       1024
#define NUM_ROWS_OR_COLS  146
#define CELL_SIZE         ((int)(SCREEN_SIZE / NUM_ROWS_OR_COLS) - 1)
#define PAD               (int)(SCREEN_SIZE % NUM_ROWS_OR_COLS)

#define MIN_SATURATION 0.4
#define MIN_VALUE      0.32

#define MAX_COLORS_IN_PALETTE 128

typedef struct {
	int id;
	int index;
	int num_blocks;
} DiskEntry;

typedef struct {
	BUF(DiskEntry *free);
	BUF(DiskEntry *files);
	BUF(int *blocks);
} Disk;

typedef struct {
	U32 *colors;
	int num_colors;
} Palette;

void part_one(Disk disk) {
	S64 checksum = 0;

	for (int i=0; i<buf_len(disk.free); ++i) {
		DiskEntry *free = &disk.free[i];
		if (free->index >= buf_len(disk.blocks) - 1) break;
		while (free->num_blocks > 0) {
			int total_blocks = buf_len(disk.blocks);
			int last_index = total_blocks - 1;
			while (last_index > 0 && disk.blocks[last_index] == -1) {
			   	--last_index;
			}
			if (last_index <= 0) break;
			disk.blocks[free->index] = disk.blocks[last_index];
			buf_set_len(disk.blocks, last_index);
			++free->index;
			--free->num_blocks;
		}	
	}

	for (int i=0; i<buf_len(disk.blocks); ++i) {
		checksum += i * disk.blocks[i];
	}

	printf("part one: %lld\n", checksum);
}

void print_blocks(Disk disk) {
	for (int i=0; i<buf_len(disk.blocks); ++i) {
		printf("%c", disk.blocks[i] == -1 ? '.' : disk.blocks[i] + '0');
	}
	printf("\n");
}

// index and num_blocks represent newly freed space, this function only 
// coalesces this newly freed space with the surrounding free blocks
// NOT all free space in disk.blocks
void coalesce_free_space(Disk disk, int index, int num_blocks) {
	if (disk.blocks[index] != -1) return;

	DiskEntry *before_free = NULL;
	for (int i=0; i<buf_len(disk.free); ++i) {
		DiskEntry *free = &disk.free[i];

		// check before
		if (free->index + free->num_blocks == index) {
			free->num_blocks += num_blocks;
			before_free = free;
		}

		// check after
		if (free->index == index + num_blocks) {
			if (before_free) {
				before_free->num_blocks += free->num_blocks;
				int len = buf_len(disk.free);
				disk.free[i] = disk.free[len - 1];
				buf_set_len(disk.free, len - 1);
			} else {
				free->index -= num_blocks;
				free->num_blocks += num_blocks;
			}
		}
	}
}


void part_two(Disk disk) {
	S64 checksum = 0;

	for (int file_index = buf_len(disk.files) - 1;
		 file_index >= 0;
		 --file_index)
	{
		DiskEntry file = disk.files[file_index];
		for (int free_index = 0;
			 free_index < buf_len(disk.free);
			 ++free_index)
		{
			DiskEntry *free = &disk.free[free_index];
			if (free->index >= file.index) continue;
			if (free->num_blocks >= file.num_blocks) {
				for (int j=0; j<file.num_blocks; ++j) {
					disk.blocks[free->index] = file.id;
					++free->index;
					--free->num_blocks;

					// NOTE(shaw): here file.index is index in disk.block
					// where file_index is index in disk.files
					disk.blocks[file.index + j] = -1;
				}	
				if (file_index == buf_len(disk.files) - 1) {
					int new_len = buf_len(disk.blocks) - file.num_blocks;
					buf_set_len(disk.blocks, new_len);
				} else {
					coalesce_free_space(disk, file.index, file.num_blocks);
				}
				break;
			}
		}
	}

	for (int i=0; i<buf_len(disk.blocks); ++i) {
		if (disk.blocks[i] != -1) {
			checksum += i * disk.blocks[i];
		}
	}

	printf("part two: %lld\n", checksum);
}


Disk disk_copy(Disk source) {
	Disk dest = {0};
	for (int i=0; i<buf_len(source.free); ++i) {
		buf_push(dest.free, source.free[i]);
	}
	for (int i=0; i<buf_len(source.files); ++i) {
		buf_push(dest.files, source.files[i]);
	}
	for (int i=0; i<buf_len(source.blocks); ++i) {
		buf_push(dest.blocks, source.blocks[i]);
	}
	return dest;
}

// h, s, and v are all expected to be between 0 and 1
U32 rgb_from_hsv(F32 h, F32 s, F32 v) {
	F32 hex = h * 6.0f;
	U8 primary = (U8)hex;
	F32 secondary = hex - primary;
	F32 x = (1.0 - s) * v;
	F32 y = (1.0 - (s * secondary)) * v;
	F32 z = (1.0 - (s * (1.0 - secondary))) * v;

	U8 r, g, b;

	switch(primary) {
	case 0:
		r = (v * 255.0) + 0.5;
		g = (z * 255.0) + 0.5;
		b = (x * 255.0) + 0.5;
		break;
	case 1:
		r = (y * 255.0) + 0.5;
		g = (v * 255.0) + 0.5;
		b = (x * 255.0) + 0.5;
		break;
	case 2:
		r = (x * 255.0) + 0.5;
		g = (v * 255.0) + 0.5;
		b = (z * 255.0) + 0.5;
		break;
	case 3:
		r = (x * 255.0) + 0.5;
		g = (y * 255.0) + 0.5;
		b = (v * 255.0) + 0.5;
		break;
	case 4:
		r = (z * 255.0) + 0.5;
		g = (x * 255.0) + 0.5;
		b = (v * 255.0) + 0.5;
		break;
	case 5:
		r = (v * 255.0) + 0.5;
		g = (x * 255.0) + 0.5;
		b = (y * 255.0) + 0.5;
		break;
	default: assert(0);
	}

	return 0xFF000000 | b << 16 | g << 8 | r;
}

Palette generate_palette(int num_colors) {
	Palette pal = {0};
	num_colors = MIN(num_colors, MAX_COLORS_IN_PALETTE);
	pal.colors = xmalloc(num_colors * sizeof(U32));
	pal.num_colors = num_colors;

	for (int i=0; i<num_colors; ++i) {
		F32 hue = rand_f32();
		F32 sat = rand_f32() * (1 - MIN_SATURATION) + MIN_SATURATION;
		F32 val = rand_f32() * (1 - MIN_VALUE) + MIN_VALUE;
		pal.colors[i] = rgb_from_hsv(hue, sat, val);
	}

	return pal;
}

void draw_disk(Disk disk, Palette palette) {
	for (int i=0; i<buf_len(disk.blocks); ++i) {
		int file_id = disk.blocks[i];
		if (file_id == -1) continue;
		int x = (i % NUM_ROWS_OR_COLS) * (CELL_SIZE+1) + PAD;
		int y = (i / NUM_ROWS_OR_COLS) * (CELL_SIZE+1) + PAD;
		U32 color = palette.colors[file_id % MAX_COLORS_IN_PALETTE];
		drawbox(x, y, CELL_SIZE, CELL_SIZE, color);
	}
}

Disk parse_partial_disk(char *file_data, int max_blocks) {
	Disk disk = {0};

	int file_id = 0;
	for (int i=0; file_data[i] != '\0'; ++i) {
		if (!isdigit(file_data[i])) continue;
		int block_val = -1;
		int num_blocks = (int)file_data[i] - '0';
		if (buf_len(disk.blocks) + num_blocks > max_blocks) {
			break;
		}

		DiskEntry entry = {0};
		entry.index = buf_len(disk.blocks);
		entry.num_blocks = num_blocks;

		if ((i % 2) == 0) {
			block_val = file_id++;
			entry.id = block_val;
			buf_push(disk.files, entry);
		} else {
			if (num_blocks == 0) continue;
			buf_push(disk.free, entry);
		}

		for (int j=0; j<num_blocks; ++j) {
			buf_push(disk.blocks, block_val);
		}
	}

	return disk;
}

void visualize(char *file_data) {
	setupgif(0, 1, "disk.gif");

	int max_blocks = NUM_ROWS_OR_COLS * NUM_ROWS_OR_COLS;
	Disk disk = parse_partial_disk(file_data, max_blocks);

	Palette palette = generate_palette(buf_len(disk.files));

	int frames = 0;
	int skip_frames = 5;
	for (int file_index = buf_len(disk.files) - 1;
		 file_index >= 0;
		 --file_index)
	{
		clear();
		draw_disk(disk, palette);
		if ((frames++ % skip_frames) == 0)
			nextframe();

		if (file_index <= 1000) skip_frames = 50;

		DiskEntry file = disk.files[file_index];
		for (int free_index = 0;
			 free_index < buf_len(disk.free);
			 ++free_index)
		{
			DiskEntry *free = &disk.free[free_index];
			if (free->index >= file.index) continue;
			if (free->num_blocks >= file.num_blocks) {
				for (int j=0; j<file.num_blocks; ++j) {
					disk.blocks[free->index] = file.id;
					++free->index;
					--free->num_blocks;

					// NOTE(shaw): here file.index is index in disk.block
					// where file_index is index in disk.files
					disk.blocks[file.index + j] = -1;
				}	
				if (file_index == buf_len(disk.files) - 1) {
					int new_len = buf_len(disk.blocks) - file.num_blocks;
					buf_set_len(disk.blocks, new_len);
				} else {
					coalesce_free_space(disk, file.index, file.num_blocks);
				}
				break;
			}
		}
	}
	
	clear();
	draw_disk(disk, palette);
	for (int i=0; i<10; ++i)
		nextframe();

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

	Disk disk = {0};

	int file_id = 0;
	for (int i=0; file_data[i] != '\0'; ++i) {
		if (!isdigit(file_data[i])) continue;
		int block_val = -1;
		int num_blocks = (int)file_data[i] - '0';

		DiskEntry entry = {0};
		entry.index = buf_len(disk.blocks);
		entry.num_blocks = num_blocks;

		if ((i % 2) == 0) {
			block_val = file_id++;
			entry.id = block_val;
			buf_push(disk.files, entry);
		} else {
			if (num_blocks == 0) continue;
			buf_push(disk.free, entry);
		}

		for (int j=0; j<num_blocks; ++j) {
			buf_push(disk.blocks, block_val);
		}
	}

	Disk disk2 = disk_copy(disk);
	part_one(disk);
	part_two(disk2);

	visualize(file_data);

	return 0;
}

