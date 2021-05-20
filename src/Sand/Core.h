#pragma once
#include <stdint.h>

//Assertions
#ifdef SAND_DEBUG
#define Assert(Expression) if(!(Expression)) {*(i8*)0 = 0;}
#else
#define Assert(Expression) 
#endif

#define internal static //static functions
#define local_persist static //static local variables
#define global static//static global variables

//Number of elements in an array
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

//math
#define Pi 3.14159265359f

//Data types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef int32_t b32;

//Numbers
#define KILOBYTES(value) (value*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)
#define TERABYTES(value) (GIGABYTES(value)*1024LL)

//Utilities
u32 TruncateU64(u64 value)
{
	u64 u64MaxValue = 0xFFFFFFFF;
	Assert(value <= u64MaxValue);
	u32 Result = (u32)value;
	return Result;
}