#include "Game.h"

#include <cmath>

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
		State->green = 255;
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
			}

			else if(Controller0->DPadLeft.EndedPress)
			{
			}

			else if(Controller0->DPadRight.EndedPress)
			{
			}

			else if(Controller0->DPadDown.EndedPress)
			{
			}
}
	}
	//Audio
	if(!SoundOutput->IsBufferFilled)
	{
		GameGenerateAudio(SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}
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