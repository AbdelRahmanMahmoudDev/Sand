#include "Game.h"

#include <cmath>

/*
Pitch is the amount of pixels needed to step through to get to the next row
It is equal to the width of the bitmap multiplied by the amount of bytes per pixel (for RGBA, 4 bytes)
*/
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

/*
The game layer is currently only responsible for filling the sound buffer
Sound play and update is managed by the OS currently
*/
extern "C" GAME_GENERATE_AUDIO(GameGenerateAudio)
{
	#if 0
    i16 amplitude = 32760;
	f32 frequency = 441.0f;
	double t = (2 * Pi * frequency) / (SoundOutput->SamplesPerSecond * SoundOutput->NumberOfChannels); //wave phase
	for(i32 sampleIndex = 0; sampleIndex < SoundOutput->SoundBufferSize; ++sampleIndex)
	{
		SoundOutput->SoundData[sampleIndex] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
		SoundOutput->SoundData[sampleIndex+1] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
	}
	#endif
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

		//File i/o
		#ifdef SAND_INTERNAL
		char* FileName = __FILE__;
		DebugPlatformReadFileResult File = Memory->DebugPlatformReadEntireFile(&thread, FileName);
		if(File.Content)
		{
		Memory->DebugPlatformWriteEntireFile(&thread, "test.out", File.Content, File.ContentSize);
		Memory->DebugPlatformFreeFileMemory(&thread, File.Content);
		}
		#endif

		//Initialize state
		State->BlueOffset = 0;
		State->GreenOffset = 0;
		Memory->IsInitialized = true;
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
	Render(BackBuffer, State->BlueOffset, State->GreenOffset);
}