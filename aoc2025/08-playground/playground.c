#include "../common.c"
#include <float.h> // for FLT_MAX

typedef struct {
	F32 x, y, z;
} Vec3;

typedef struct Junction Junction;
struct Junction {
	Vec3 pos;
	BUF(int *connections);
};

Vec3 vec3_sub(Vec3 a, Vec3 b) {
	return (Vec3) {
		.x = a.x - b.x,
		.y = a.y - b.y,
		.z = a.z - b.z,
	};
}

// square magnitude
F32 vec3_mag2(Vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

// square distance
F32 vec3_dist2(Vec3 a, Vec3 b) {
	return vec3_mag2(vec3_sub(b, a));
}

bool is_connected(BUF(Junction *junctions), int a, int b) {
	BUF(int *connections) = junctions[a].connections;
	for (int i=0; i<buf_len(connections); ++i) {
		if (connections[i] == b) return true;
	}
	return false;
}

void ciruit_length_traverse(BUF(Junction *junctions), size_t id, Map *visited, Map *external_visited) {
	for (int i=0; i<buf_len(junctions[id].connections); ++i) {
		size_t other = junctions[id].connections[i];
		// NOTE: add one because hash map cannot store 0 keys
		bool seen = (bool)map_get(visited, (void*)(other+1)); 
		if (!seen) {
			map_put(visited, (void*)(other+1), (void*)1);
			map_put(external_visited, (void*)(other+1), (void*)1);
			ciruit_length_traverse(junctions, other, visited, external_visited);
		}
	}
}

int circuit_length(BUF(Junction *junctions), size_t id, Map *external_visited) {
	Map visited = {0};
	// NOTE: add one because hash map cannot store 0 values
	map_put(&visited, (void*)(id+1), (void*)1);
	map_put(external_visited, (void*)(id+1), (void*)1);
	ciruit_length_traverse(junctions, id, &visited, external_visited);
	int length = (int)visited.len;
	map_clear(&visited);
	return length;
}

void part_one(BUF(Junction *junctions)) {
	int max_connections = buf_len(junctions) < 1000 ? 10 : 1000;
	for (int k=0; k<max_connections; ++k) {
		F32 min_dist = FLT_MAX;
		int min_a, min_b;
		for (int i=0; i<buf_len(junctions)-1; ++i) {
			for (int j=i+1; j<buf_len(junctions); ++j) {
				F32 dist = vec3_dist2(junctions[i].pos, junctions[j].pos);
				if (dist < min_dist && !is_connected(junctions, i, j)) {
					min_dist = dist;
					min_a = i;
					min_b = j;
				}
			}
		}

		assert(min_dist < FLT_MAX);
		buf_push(junctions[min_a].connections, min_b);
		buf_push(junctions[min_b].connections, min_a);
	}

	// find 3 largest circuits
	// multiply sizes 
	int len_3 = 0;
	int len_2 = 0;
	int len_1 = 0;
	Map visited = {0};
	for (size_t i=0; i<buf_len(junctions); ++i) {
		if (map_get(&visited, (void*)(i+1)) == 0) {
			int len = circuit_length(junctions, i, &visited);
			if (len > len_3) {
				len_1 = len_2;
				len_2 = len_3;
				len_3 = len;
			} else if (len > len_2) {
				len_1 = len_2;
				len_2 = len;
			} else if (len > len_1) {
				len_1 = len;
			}
		}
	}

	map_clear(&visited);
	printf("part one: %d\n", len_3 * len_2 * len_1);
}

void part_two(BUF(Junction *junctions)) {
	int result = 0;
	// printf("part two: %d\n", result);
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

	BUF(Junction *junctions) = NULL;
	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == '\r' || *line == 0) break;
		Junction j = {0};
		sscanf(line, "%f,%f,%f", &j.pos.x, &j.pos.y, &j.pos.z);
		buf_push(junctions, j);
	}

	part_one(junctions);
	part_two(junctions);

	return 0;
}
