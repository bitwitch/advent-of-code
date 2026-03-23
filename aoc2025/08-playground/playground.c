#include "../common.c"
#include <float.h> // for FLT_MAX

typedef struct {
	F32 x, y, z;
} Vec3;

typedef struct {
	Vec3 pos;
	BUF(int *connections);
} Junction;

typedef struct {
	F32 dist;
	int id_a, id_b;
} Distance;

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
			if (external_visited) {
				map_put(external_visited, (void*)(other+1), (void*)1);
			}
			
			ciruit_length_traverse(junctions, other, visited, external_visited);
		}
	}
}

int circuit_length(BUF(Junction *junctions), size_t id, Map *external_visited) {
	Map visited = {0};
	// NOTE: add one because hash map cannot store 0 values
	map_put(&visited, (void*)(id+1), (void*)1);
	if (external_visited) {
		map_put(external_visited, (void*)(id+1), (void*)1);
	}
	ciruit_length_traverse(junctions, id, &visited, external_visited);
	int length = (int)visited.len;
	map_clear(&visited);
	return length;
}

void junction_print(Junction j, int id) {
	printf("%4d (%.0f, %.0f, %.0f) %d connections [", id, j.pos.x, j.pos.y, j.pos.z, buf_len(j.connections));
	for (int i=0; i<buf_len(j.connections); ++i) {
		if (i > 0) printf(", ");
		printf("%d", j.connections[i]);
	}
	printf("]\n");
}

void part_one(BUF(Junction *junctions), BUF(Distance *distances)) {
	int max_connections = buf_len(junctions) < 1000 ? 10 : 1000;
	int i=0;
	for (int k=0; k<max_connections; ++k) {
		Distance d = distances[i];
		while (i < buf_len(distances) && is_connected(junctions, d.id_a, d.id_b)) {
			i += 1;
			d = distances[i];
		}
		buf_push(junctions[d.id_a].connections, d.id_b);
		buf_push(junctions[d.id_b].connections, d.id_a);
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

void part_two(BUF(Junction *junctions), BUF(Distance *distances)) {
	int result = 0;
	for (int i=0; i < buf_len(distances); ++i) {
		Distance d = distances[i];
		if (!is_connected(junctions, d.id_a, d.id_b)) {
			buf_push(junctions[d.id_a].connections, d.id_b);
			buf_push(junctions[d.id_b].connections, d.id_a);
			int len = circuit_length(junctions, d.id_a, NULL);
			if (len == buf_len(junctions)) {
				result = (int)junctions[d.id_a].pos.x * (int)junctions[d.id_b].pos.x;
				break;
			}
		}
	}

	printf("part two: %d\n", result);
}

BUF(Distance *) calculate_distances(BUF(Junction *junctions)) {
	BUF(Distance *distances) = NULL;
	int num_junctions = buf_len(junctions);
	buf_set_cap(distances, num_junctions*num_junctions);

	for (int i=0; i<num_junctions-1; ++i) {
		for (int j=i+1; j<num_junctions; ++j) {
			Distance d = {
				.dist = vec3_dist2(junctions[i].pos, junctions[j].pos),
				.id_a = i,
				.id_b = j,
			};
			buf_push(distances, d);
		}
	}
	return distances;
}

int cmp_distances(const void *_a, const void *_b) {
	Distance *a = (Distance*)_a;
	Distance *b = (Distance*)_b;
	if (a->dist < b->dist) return -1;
	if (a->dist > b->dist) return 1;
	return 0;
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
	BUF(Junction *junctions_copy) = NULL;
	for (;;) {
		char *line = chop_by_delimiter(&file_data, "\n");
		if (*line == '\r' || *line == 0) break;
		Junction j = {0};
		sscanf(line, "%f,%f,%f", &j.pos.x, &j.pos.y, &j.pos.z);
		buf_push(junctions, j);
		Junction j2 = j;
		buf_push(junctions_copy, j2);
	}

	BUF(Distance *distances) = calculate_distances(junctions);
	qsort(distances, buf_len(distances), sizeof(distances[0]), cmp_distances);

	part_one(junctions, distances);
	part_two(junctions_copy, distances);

	return 0;
}
