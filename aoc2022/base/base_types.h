#pragma once

/*******************************
 * Primative Types
 *******************************/
#include <stdint.h>
#include <stdbool.h>
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


/*******************************
 * Helper Macros
 *******************************/
#define global static
#define local_persist static
#define function static

#define Stmnt(S) do{ S }while(0)

#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
#define Clamp(a,x,b) (((x)<(a))?(a):\
((b)<(x))?(b):(x))
#define ClampTop(a,b) Min(a,b)
#define ClampBot(a,b) Max(a,b)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)
