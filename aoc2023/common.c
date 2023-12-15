#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ARRAY_COUNT(a) sizeof(a)/sizeof(*(a))

#ifdef ORCA_MODE
#include "common_orca.c"
#else
#include "common_native.c"
#endif


