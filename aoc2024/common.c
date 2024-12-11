#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ARRAY_COUNT(a) sizeof(a)/sizeof(*(a))
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define IS_SET(bits, check) ((bits & check) == check)

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t  S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;

typedef float  F32;
typedef double F64;

#if _WIN32
U64 os_file_size(char *filepath) {
	struct __stat64 stat = {0};
	_stat64(filepath, &stat);
	return stat.st_size;
}
#else
#error os_file_size not defined on this platform
#endif


// ---------------------------------------------------------------------------
// Helper Utilities
// ---------------------------------------------------------------------------
void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        perror("malloc");
        exit(1);
    }
    return ptr;
}

void *xcalloc(size_t num_items, size_t item_size) {
    void *ptr = calloc(num_items, item_size);
    if (ptr == NULL) {
        perror("calloc");
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *result = realloc(ptr, size);
    if (result == NULL) {
        perror("recalloc");
        exit(1);
    }
    return result;
}

void fatal(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("FATAL: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    exit(1);
}

char *chop_by_delimiter(char **str, char *delimiter) {
    char *chopped = *str;

    char *found = strstr(*str, delimiter);
    if (found == NULL) {
        *str += strlen(*str);
        return chopped;
    }

    *found = '\0';
    *str = found + strlen(delimiter);

    return chopped;
}

bool read_entire_file(char *filepath, char **file_data, size_t *out_size) {
	FILE *f = fopen(filepath, "rb");
	if (!f) {
		return false;
	}

	U64 file_size = os_file_size(filepath);

	*out_size = file_size + 1;
	*file_data = malloc(*out_size);
	if (!*file_data) {
		fclose(f);
		return false;
	}

	size_t bytes_read = fread(*file_data, 1, file_size, f);
	if (bytes_read < file_size && !feof(f)) {
		fclose(f);
		return false;
	}

	(*file_data)[bytes_read] = 0; // add null terminator
	fclose(f);

	return true;
}

// ---------------------------------------------------------------------------
// Stretchy Buffers, a la sean barrett
// ---------------------------------------------------------------------------

typedef struct {
	size_t len;
	size_t cap;
	char buf[]; // flexible array member
} BUF_Header;


// get the metadata of the array which is stored before the actual buffer in memory
#define buf__header(b) ((BUF_Header*)((char*)b - offsetof(BUF_Header, buf)))
// checks if n new elements will fit in the array
#define buf__fits(b, n) (buf_lenu(b) + (n) <= buf_cap(b)) 
// if n new elements will not fit in the array, grow the array by reallocating 
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = buf__grow((b), buf_lenu(b) + (n), sizeof(*(b)))))

#define BUF(x) x // annotates that x is a stretchy buffer
#define buf_len(b)  ((b) ? (int32_t)buf__header(b)->len : 0)
#define buf_lenu(b) ((b) ?          buf__header(b)->len : 0)
#define buf_set_len(b, l) buf__header(b)->len = (l)
#define buf_cap(b) ((b) ? buf__header(b)->cap : 0)
#define buf_end(b) ((b) + buf_lenu(b))
#define buf_push(b, ...) (buf__fit(b, 1), (b)[buf__header(b)->len++] = (__VA_ARGS__))
#define buf_free(b) ((b) ? (free(buf__header(b)), (b) = NULL) : 0)
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))

void *buf__grow(void *buf, size_t new_len, size_t elem_size) {
	size_t new_cap = MAX(1 + 2*buf_cap(buf), new_len);
	assert(new_len <= new_cap);
	size_t new_size = offsetof(BUF_Header, buf) + new_cap*elem_size;

	BUF_Header *new_header;
	if (buf) {
		new_header = xrealloc(buf__header(buf), new_size);
	} else {
		new_header = xmalloc(new_size);
		new_header->len = 0;
	}
	new_header->cap = new_cap;
	return new_header->buf;
}

// ---------------------------------------------------------------------------
// Arena Allocator
// ---------------------------------------------------------------------------
#define ARENA_BLOCK_SIZE 65536

typedef struct {
    char *ptr;
    char *end;
    BUF(char **blocks);
} Arena;

void arena_grow(Arena *arena, size_t min_size) {
    size_t size = MAX(ARENA_BLOCK_SIZE, min_size);
    arena->ptr = xmalloc(size);
    arena->end = arena->ptr + size;
    buf_push(arena->blocks, arena->ptr);
}

void *arena_alloc(Arena *arena, size_t size) {
    if (arena->ptr + size > arena->end) {
        arena_grow(arena, size); 
    }
    void *ptr = arena->ptr;
    arena->ptr += size;
    return ptr;
}

void *arena_alloc_zeroed(Arena *arena, size_t size) {
    void *ptr = arena_alloc(arena, size);
    memset(ptr, 0, size);
    return ptr;
}

void arena_free(Arena *arena) {
    for (int i=0; i<buf_len(arena->blocks); ++i) {
        free(arena->blocks[i]);
    }
    buf_free(arena->blocks);
}

void *arena_memdup(Arena *arena, void *src, size_t size) {
    if (size == 0) return NULL;
    void *new_mem = arena_alloc(arena, size);
    memcpy(new_mem, src, size);
    return new_mem;
}

// ---------------------------------------------------------------------------
// Hash Map
// ---------------------------------------------------------------------------

typedef struct {
	void **keys;
	void **vals;
	size_t len;
	size_t cap;
} Map;

uint64_t uint64_hash(uint64_t x) {
	x ^= (x * 0xff51afd7ed558ccdull) >> 32;
	return x;
}

uint64_t ptr_hash(void *ptr) {
	return uint64_hash((uintptr_t)ptr);
}

uint64_t str_hash_range(char *start, char *end) {
	uint64_t fnv_init = 0xcbf29ce484222325ull;
	uint64_t fnv_prime = 0x00000100000001B3ull;
	uint64_t hash = fnv_init;
	while (start != end) {
		hash ^= *start++;
		hash *= fnv_prime;
		hash ^= hash >> 32; // additional mixing
	}
	return hash;
}

void *map_get(Map *map, void *key) {
	if (map->len == 0) {
		return NULL;
	}
	assert(map->len < map->cap);
	size_t i = (size_t)ptr_hash(key);

	for (;;) {
		i &= map->cap - 1; // power of two masking
		if (map->keys[i] == NULL) 
			return NULL;
		if (map->keys[i] == key)
			return map->vals[i];
		++i;
	}
}

void map_put(Map *map, void *key, void *val);

void map_grow(Map *map, size_t new_cap) {
	new_cap = MAX(16, new_cap);
	assert(IS_POW2(new_cap));
	Map new_map = {
		.keys = xcalloc(new_cap, sizeof(void*)),
		.vals = xmalloc(new_cap * sizeof(void*)),
		.cap = new_cap,
	};

	for (size_t i = 0; i < map->cap; ++i) {
		if (map->keys[i]) {
			map_put(&new_map, map->keys[i], map->vals[i]);
		}
	}

	free(map->keys);
	free(map->vals);

	*map = new_map;
}

void map_put(Map *map, void *key, void *val) {
	assert(key && val);
	// TODO(shaw): currently enforcing less than 50% capacity, tweak this to be
	// less extreme/conservative
	if (2*map->len >= map->cap) {
		map_grow(map, 2*map->cap);
	}
	assert(2*map->len < map->cap); 

	size_t i = (size_t)ptr_hash(key);

	for (;;) {
		i &= map->cap - 1;
		if (map->keys[i] == NULL) { 
			map->keys[i] = key;
			map->vals[i] = val;
			++map->len;
			return;
		}
		if (map->keys[i] == key) {
			map->vals[i] = val;
			return;
		}
		++i;
	}

}

void map_clear(Map *map) {
	free(map->keys);
	free(map->vals);
	memset(map, 0, sizeof(*map));
}

void map_test(void) {
	Map map = {0};
	enum { N = 1024 * 1024 };
	for (size_t i=0; i<N; ++i) {
		map_put(&map, (void*)(i+1), (void*)(i+2));
	}
	for (size_t i=0; i<N; ++i) {
		assert(map_get(&map, (void*)(i+1)) == (void*)(i+2));
	}
}


// ---------------------------------------------------------------------------
// String Interning
// ---------------------------------------------------------------------------
typedef struct InternStr InternStr;
struct InternStr {
    size_t len;
	InternStr *next;
    char str[];
};

static Arena intern_arena;
static Map interns;

char *str_intern_range(char *start, char *end) {
	size_t len = end - start;
	uint64_t hash = str_hash_range(start, end);
	void *key = (void*)(uintptr_t)(hash ? hash : 1);

	// check if string is already interned
	InternStr *intern = map_get(&interns, key);
	// NOTE: in almost all cases this loop should only execute a single time
	for (InternStr *it = intern; it; it = it->next) {
		if (it->len == len && strncmp(it->str, start, len) == 0) {
			return it->str;
		} 
	}

	InternStr *new_intern = arena_alloc(&intern_arena, offsetof(InternStr, str) + len + 1);
	new_intern->len = len;
	new_intern->next = intern;
	memcpy(new_intern->str, start, len);
	new_intern->str[len] = 0;
	map_put(&interns, key, new_intern);
	return new_intern->str;
}

char *str_intern(char *str) {
    return str_intern_range(str, str + strlen(str));
}


// PCG random number generator taken from https://en.wikipedia.org/wiki/Permuted_congruential_generator
static U64 _rand_state            = 0x4d595df4d0f33173;   // Or something seed-dependent
static U64 const _rand_multiplier = 6364136223846793005u;
static U64 const _rand_increment  = 1442695040888963407u;	// Or an arbitrary odd constant

static U32 rotr32(U32 x, U32 r) {
	return x >> r | x << ((U32)(-(S32)r) & 31);
}

U32 pcg32(void) {
	U64 x = _rand_state;
	unsigned count = (unsigned)(x >> 59);   // 59 = 64 - 5

	_rand_state = x * _rand_multiplier + _rand_increment;
	x ^= x >> 18;                           // 18 = (64 - 27)/2
	return rotr32((U32)(x >> 27), count);   // 27 = 32 - 5
}

void pcg32_init(U64 seed) {
	_rand_state = seed + _rand_increment;
	(void)pcg32();
}

U32 rand_range_u32(U32 min, U32 max) {
	U32 range = max - min + 1;
	return min + (pcg32() % range);
}

F32 rand_f32(void) {
	return (F32)((F64)pcg32() / (F64)UINT32_MAX);
}

