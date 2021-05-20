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
		//File i/o
		#ifdef SAND_INTERNAL
		char* FileName = __FILE__;
		DebugPlatformReadFileResult File = Memory->DebugPlatformReadEntireFile(FileName);
		if(File.Content)
		{
		Memory->DebugPlatformWriteEntireFile("test.out", File.Content, File.ContentSize);
		Memory->DebugPlatformFreeFileMemory(File.Content);
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
	//Audio
	if(!SoundOutput->IsBufferFilled)
	{
		GameGenerateAudio(SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}

	//Graphics
	Render(BackBuffer, State->BlueOffset, State->GreenOffset);
	//++State->BlueOffset;
	//++State->GreenOffset;
#if 0

	OpenGLRendererState RendererState = {};
	if(!RendererState.IsRendererPrepared)
	{
		RendererState = OpenGLRendererInit();
		RendererState.IsRendererPrepared = true;
	}
	
	glUseProgram(RendererState.ShaderProgram);
	glBindVertexArray(RendererState.VertexArrayHandle);
	glDrawArrays(GL_TRIANGLES, 0, 3);
    //Rendering
#endif
}

#if SAND_WIN32
#include "Windows.h"
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
	return TRUE;
}

#endif