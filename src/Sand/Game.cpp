#include "Game.h"

#include <cmath>

internal i32 Roundf32Toi32(f32 value)
{
	i32 Result = (i32)(value + 0.5f);
	return Result;
}

internal void DrawRectangle(GameBackBuffer* Buffer, f32 MinX, f32 MaxX, f32 MinY, f32 MaxY, u32 Color)
{
	i32 iMinX = Roundf32Toi32(MinX);
	i32 iMaxX = Roundf32Toi32(MaxX);
	i32 iMinY = Roundf32Toi32(MinY);
	i32 iMaxY = Roundf32Toi32(MaxY);

	if(iMinX < 0)
	{
		iMinX = 0;
	}

	if(iMaxX > Buffer->BitmapWidth)
	{
		iMaxX = Buffer->BitmapWidth;
	}

	if(iMinY < 0)
	{
		iMinY = 0;
	}

	if(iMaxY > Buffer->BitmapHeight)
	{
		iMaxY = Buffer->BitmapHeight;	
	}


	u8* Row = (u8*)Buffer->BitmapMemory + (iMinY * Buffer->Pitch) + (iMinX * Buffer->BytesPerPixel);
	for(i32 Y = 0; Y < iMaxY; ++Y)
	{
		u32* Pixel = (u32*)Row;
		for(i32 X = 0; X < iMaxX; ++X)
		{
			*Pixel++ = Color;
		}
		Row+= Buffer->Pitch;
	}
}

/*
The game layer is currently only responsible for filling the sound buffer
Sound play and update is managed by the OS currently
*/
extern "C" GAME_GENERATE_AUDIO(GameGenerateAudio)
{
    i16 amplitude = 32760;
	f32 frequency = 441.0f;
	double t = (2 * Pi * frequency) / (SoundOutput->SamplesPerSecond * SoundOutput->NumberOfChannels); //wave phase
	for(i32 sampleIndex = 0; sampleIndex < SoundOutput->SoundBufferSize; ++sampleIndex)
	{
	#if 0
		SoundOutput->SoundData[sampleIndex] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
		SoundOutput->SoundData[sampleIndex+1] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
	#else
		SoundOutput->SoundData[sampleIndex] = 0;
	#endif
	}
}
extern "C" GAME_UPDATE(GameUpdate)
{
	//Memory
	Assert(sizeof(GameState) <= Memory->PermanentStorageSize);
	GameState* State = (GameState*)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		//Placeholder thread thing
		ThreadContext thread = {};
		Memory->IsInitialized = true;
		int foo;
	}

	//Input
	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		GameController* Controller0 = &Input->Controllers[ControllerIndex];
		if(Controller0->IsAnalog)
		{
			//Analog processing
		}
		else
		{
			//Digital processing
			if(Controller0->DPadUp.EndedPress)
			{
				--State->GreenOffset;
			}

			else if(Controller0->DPadLeft.EndedPress)
			{
				--State->BlueOffset;
			}

			else if(Controller0->DPadRight.EndedPress)
			{
				++State->BlueOffset;
			}

			else if(Controller0->DPadDown.EndedPress)
			{
				++State->GreenOffset;
			}
		}
	}

	if(Input->MouseButtons[0].EndedPress)
	{
		--State->BlueOffset;
	}
	//Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}

	//Graphics

	DrawRectangle(BackBuffer, 0.0f, (f32)BackBuffer->BitmapWidth,
				  0.0f, (f32)BackBuffer->BitmapHeight, 0x00FF00FF);
				  
	DrawRectangle(BackBuffer, 10.0f, 10.0f,
				  10.0f, 20.0f, 0xFFFFFFFF);
}

/*
Pitch is the amount of pixels needed to step through to get to the next row
It is equal to the width of the bitmap multiplied by the amount of bytes per pixel (for RGBA, 4 bytes)
internal void Render(GameBackBuffer* BackBuffer, i32 BlueOffset, i32 GreenOffset)
{
	//i32 Width = BackBuffer->BitmapWidth;

	
	//i32 BackBuffer->Pitch = Width * BackBuffer->BytesPerPixel;

	u8* Row = (u8*)BackBuffer->BitmapMemory;

	for(i32 Y = 0; Y < BackBuffer->BitmapHeight; ++Y)
	{
		u32* Pixel = (u32*)Row;
		for(i32 X = 0; X < BackBuffer->BitmapWidth; ++X)
		{
			u8 Blue = (u8)(X + BlueOffset);
			u8 Green = (u8)(Y + GreenOffset);

			*Pixel++ = ((Green << 8) | Blue); 
		}
		Row += BackBuffer->Pitch;
	}
}
*/