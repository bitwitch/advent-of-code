#include "../common.c"

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

	return 0;
}

