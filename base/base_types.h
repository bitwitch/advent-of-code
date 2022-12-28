#pragma once

#include "stb_ds.h"

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


/*******************************
 * Linked List Macros
 *******************************/
#define SLLQueuePush_N(f,l,n,next) ((f)==0?\
((f)=(l)=(n)):\
(((l)->next=(n)),((l)=(n))),\
((n)->next=0))

#define SLLQueuePush(f,l,n) SLLQueuePush_N(f,l,n,next)

#define SLLQueuePushFront_N(f,l,n,next) ((f)==0?\
((f)=(l)=(n),(n)->next=0):\
((n)->next=(f),(f)=(n)))

#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_N(f,l,n,next)

#define SLLQueuePop_N(f,l,next) ((f)==(l)?\
((f)=(l)=0):\
((f)=(f)->next))

#define SLLQueuePop(f,l) SLLQueuePop_N(f,l,next)

#define SLLStackPush_N(f,n,next) ((n)->next=(f),(f)=(n))

#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)

#define SLLStackPop_N(f,next) ((f)==0?0:\
(f)=(f)->next)

#define SLLStackPop(f) SLLStackPop_N(f,next)
