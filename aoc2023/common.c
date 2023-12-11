#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>

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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ARRAY_COUNT(a) sizeof(a)/sizeof(*(a))

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

