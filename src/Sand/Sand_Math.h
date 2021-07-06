#include <cmath>

internal i32 Roundf32Toi32(f32 value)
{
	i32 Result = (i32)(value + 0.5f);
	return Result;
}

internal i32 Truncatef32Toi32(f32 value)
{
	i32 Result = (i32)(value);
	return Result;
}

internal u32 Roundf32Tou32(f32 value)
{
	u32 Result = (u32)(value + 0.5f);
	return Result;
}

internal i32 Floorf32Toi32(f32 value)
{
	i32 Result = (i32)floorf(value);
	return Result;
}