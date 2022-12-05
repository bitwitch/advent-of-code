#pragma once

// Base Layer ideas from Allen Webster: https://github.com/Mr-4th-Programming/mr4th/

/*******************************
 * Context Cracking
 *******************************/
#if defined(__clang__)
# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error failed to identify OS
# endif

# if defined(__amd64__)
#  define ARCH_X64 1
// TODO(allen): verify this works on clang
# elif defined(__i386__)
#  define ARCH_X86 1
// TODO(allen): verify this works on clang
# elif defined(__arm__)
#  define ARCH_ARM 1
// TODO(allen): verify this works on clang
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error failed to identify ARCH
# endif

#elif defined(_MSC_VER)
# define COMPILER_CL 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error failed to identify OS
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_I86)
#  define ARCH_X86 1
# elif defined(_M_ARM)
#  define ARCH_ARM 1
// TODO(allen): ARM64?
# else
#  error failed to identify ARCH
# endif

#elif defined(__GNUC__)
# define COMPILER_GCC 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error failed to identify OS
# endif

# if defined(__amd64__)
#  define ARCH_X64 1
# elif defined(__i386__)
#  define ARCH_X86 1
# elif defined(__arm__)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error failed to identify ARCH
# endif

#else
# error no context cracking for this compiler
#endif

// NOTE(allen): Zero fill missing context macros
#if !defined(COMPILER_CL)
# define COMPILER_CL 0
#endif
#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif
#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined(OS_MAC)
# define OS_MAC 0
#endif
#if !defined(ARCH_X64)
# define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
# define ARCH_X86 0
#endif
#if !defined(ARCH_ARM)
# define ARCH_ARM 0
#endif
#if !defined(ARCH_ARM64)
# define ARCH_ARM64 0
#endif

typedef enum OperatingSystem OperatingSystem;
typedef enum Architecture Architecture;

enum OperatingSystem {
  OperatingSystem_Null,
  OperatingSystem_Windows,
  OperatingSystem_Linux,
  OperatingSystem_Mac,
  OperatingSystem_Count
};

enum Architecture {
  Architecture_Null,
  Architecture_X64,
  Architecture_X86,
  Architecture_Arm,
  Architecture_Arm64,
  Architecture_Count
};


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
 * Linked List Helpers
 *******************************/

// NOTE(shaw): the NP versions allow the user to supply their own names for
// next and prev pointers

#define DLLPushBack_NP(f,l,n,next,prev) ((f)==0?\
((f)=(l)=(n),(n)->next=(n)->prev=0):\
((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=0))

#define DLLPushBack(f,l,n) DLLPushBack_NP(f,l,n,next,prev)

#define DLLRemove_NP(f,l,n,next,prev) ((f)==(n)?\
((f)==(l)?\
((f)=(l)=(0)):\
((f)=(f)->next,(f)->prev=0)):\
(l)==(n)?\
((l)=(l)->prev,(l)->next=0):\
((n)->next->prev=(n)->prev,\
(n)->prev->next=(n)->next))

#define DLLRemove(f,l,n) DLLRemove_NP(f,l,n,next,prev)
