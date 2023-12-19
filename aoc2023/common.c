#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ARRAY_COUNT(a) sizeof(a)/sizeof(*(a))
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define IS_SET(bits, check) ((bits & check) == check)

#ifdef ORCA_MODE
#include "common_orca.c"
#else
#include "common_native.c"
#include "lex_native.c"
#endif


