#pragma once
#include "Core.h"

struct GameBackBuffer
{
	void* BitmapMemory;
	i32 BitmapWidth;
	i32 BitmapHeight;
	i32 BytesPerPixel;
	i32 Pitch;
};

struct GameSoundOutput
{
	u64 SoundBufferSize;
	u16 NumberOfChannels;
	u32 SamplesPerSecond;
	u32 BytesPerSample;
	u16 BitsPerSample;
	u8* SoundData;
	b32 IsBufferFilled;
};

struct GameButtonState
{
	i32 HalfTransitionCount;
	b32 EndedPress;
};

struct GameController
{
	b32 IsConnected;
	b32 IsAnalog;
    
	//Left stick
	f32 LeftStickAverageX;
	f32 LeftStickAverageY;
    
	//Right stick
	f32 RightStickAverageX;
	f32 RightStickAverageY;
    
	//Left Trigger
	f32 LeftTriggerAverage;
    
	//Right Trigger
	f32 RightTriggerAverage;
    
	union
	{
		GameButtonState Buttons[14];
		struct 
		{
            GameButtonState DPadUp;
            GameButtonState DPadDown;
            GameButtonState DPadLeft;
            GameButtonState DPadRight;
            
            GameButtonState A;
            GameButtonState B;
            GameButtonState X;
            GameButtonState Y;
            
            GameButtonState Start;
            GameButtonState Back;
            
            GameButtonState LeftShoulder;
            GameButtonState RightShoulder;
            
            GameButtonState LeftThumb;
            GameButtonState RightThumb;
		};
	};
};

struct GameInput
{
	//timestep
	f32 TargetSecondsPerFrame;
    
	//debug mouse stuff
	GameButtonState MouseButtons[5];
	i32 MouseX, MouseY, MouseZ;
	//1 keyboard + 4 game pads
	GameController Controllers[5];
};

GameController* GetController(GameInput* Input, u32 ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
	GameController* Result = &Input->Controllers[ControllerIndex]; 
	return Result;
}

struct DebugPlatformReadFileResult
{
	u32 ContentSize;
	void* Content;
};

struct ThreadContext
{
	i32 Placeholder;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DebugPlatformReadFileResult name(ThreadContext* Thread, char* FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(ThreadContext* Thread, char* FileName, void* WriteableMemory, u32 MemorySize)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext* Thread, void* FreeableMemory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

/*
Permanent Storage is data that persists between frames
Transient Storage is data that doesn't necessarily live in memory between frames (disk i/o)
IMPORTANT: All storage must be cleared to zero at startup
Debug file functions are stored here
*/
struct GameMemory
{
	b32 IsInitialized;
	u64 PermanentStorageSize;
	void* PermanentStorage;
    
	u64 TransientStorageSize;
	void* TransientStorage;
    
	debug_platform_read_entire_file* DebugPlatformReadEntireFile;
	debug_platform_write_entire_file* DebugPlatformWriteEntireFile;
	debug_platform_free_file_memory* DebugPlatformFreeFileMemory;
};

#define GAME_GENERATE_AUDIO(name) void name(ThreadContext* Thread, GameSoundOutput* SoundOutput)
typedef GAME_GENERATE_AUDIO(game_generate_audio);

#define GAME_UPDATE(name) void name(ThreadContext* Thread, GameMemory* Memory, GameBackBuffer* BackBuffer, GameInput* Input, GameSoundOutput* SoundOutput)
typedef GAME_UPDATE(game_update);

struct TileMap
{
	u32* Data;
};

struct World
{
	f32 TileWidthInMeters;
	f32 TileWidthInPixels;
	f32 MetersToPixels;

	i32 TileMapCountX;
	i32 TileMapCountY;

    f32 UpperLeftX;
	f32 UpperLeftY;
    
	f32 TileWidth;
	f32 TileHeight;
    
	i32 WidthCount;
	i32 HeightCount;
    
    TileMap *TileMaps;
};

struct CanonicalPosition
{
	//Casey says these will be packed into a single i32 with
	//some low bits representing the tile index
	//and the high bits representing the tile page
	i32 TileMapX;
	i32 TileMapY;

	i32 TileX;
	i32 TileY;

	f32 TileRelativeX;
	f32 TileRelativeY;
};

//for now, this is a dummy structure that saves state that persists between frames
struct GameState
{
	CanonicalPosition PlayerPos;
};