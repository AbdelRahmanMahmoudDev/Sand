#include "Core.h"
#include "Game.h"
#include "OpenGLCommon.h"

//#include "../Vendor/glad/glad/glad.h"

#include <stdio.h>
#include <wrl.h>
#include <Windows.h>
#include <xaudio2.h>
#include <Xinput.h>

struct Win32SoundOutput
{
	Microsoft::WRL::ComPtr<IXAudio2> XAudioObject;
	IXAudio2MasteringVoice* Win32MasterVoice;
	IXAudio2SourceVoice* Win32SourceVoice;
    XAUDIO2_BUFFER SoundBuffer;
	u64 SoundBufferSize;
	u16 NumberOfChannels;
	u32 SamplesPerSecond;
	u32 BytesPerSample;
	u16 BitsPerSample;
	u8* SoundData;
};

struct Win32BackBuffer
{

};

global b32 Win32IsRunning;
global u32 Win32KeyCode;
global b32 Win32WasAltKeyDown;
global b32 Win32WasKeyDown;
global b32 Win32IsKeyDown;
//global HDC MainDeviceContext;
global HGLRC OpenGLRenderContext;
global LARGE_INTEGER PerfFrequency;

global void* BitmapMemory;
global BITMAPINFO BitmapInfo;
global HBITMAP BitmapHandle;
global i32 BitmapWidth;
global i32 BitmapHeight;
global i32 BytesPerPixel;

typedef HGLRC WINAPI wglCreateContextAttribsARBPtr(HDC hdc, HGLRC hShareContext, const int* attribList);
wglCreateContextAttribsARBPtr* wglCreateContextAttribsARB;


typedef BOOL WINAPI wglChoosePixelFormatARBPtr(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* mNumFormats);
wglChoosePixelFormatARBPtr* wglChoosePixelFormatARB;

typedef BOOL wglSwapIntervalEXTPtr(int interval);
wglSwapIntervalEXTPtr* wglSwapIntervalEXT;

typedef int wglGetSwapIntervalEXTPtr(void);
wglGetSwapIntervalEXTPtr* wglGetSwapIntervalEXT;

typedef DWORD xinput_get_state_ptr(DWORD dwUserIndex, XINPUT_STATE *pState);
xinput_get_state_ptr* xinput_get_state;
#define XInputGetState xinput_get_state

typedef DWORD xinput_set_state_ptr(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);
xinput_set_state_ptr* xinput_set_state;
#define XInputSetState xinput_set_state

/*
Initialize OpenGL for Windows
Windows by default installs OpenGL 1.1, modern OpenGL is available on the dedicated graphics driver and must be loaded on Windows
This process, as per the documentation is:
Create a dummy render context to load the necessary extensions to create a full render context with extensions
Set the current context
*/
#if 0
internal void Win32InitOpenGL(i32 major_version, i32 minor_version)
{
	WNDCLASSEXA DummyWindow = {};
	DummyWindow.cbSize = sizeof(DummyWindow);
	DummyWindow.hInstance = GetModuleHandleW(0);
	DummyWindow.lpfnWndProc = DefWindowProcA;
	DummyWindow.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	DummyWindow.lpszClassName = "DummyWindow";
	RegisterClassExA(&DummyWindow);
	HWND DummyWindowHandle = CreateWindowExA(0, DummyWindow.lpszClassName, "SandEngine: Platform Test App", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, DummyWindow.hInstance, NULL);
	if (DummyWindowHandle)
	{
		HDC DummyDC = GetDC(DummyWindowHandle);
		HGLRC DummyOpenGLContext = 0;
		PIXELFORMATDESCRIPTOR DummyDescriptor =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			0,
			0,
			0, 0, 0
		};
		u32 pixelFormatResult = ChoosePixelFormat(DummyDC, &DummyDescriptor);
		Assert(pixelFormatResult != 0);
		u32 isPixelFormat = SetPixelFormat(DummyDC, pixelFormatResult, &DummyDescriptor);
		Assert(isPixelFormat == TRUE);
		DummyOpenGLContext = wglCreateContext(DummyDC);
		Assert(DummyOpenGLContext != NULL);
		u32 contextResult = wglMakeCurrent(DummyDC, DummyOpenGLContext);
		Assert(contextResult == TRUE);
		u32 gladStatus = gladLoadGL();
		
		Assert(gladStatus != 0);
		wglCreateContextAttribsARB = (wglCreateContextAttribsARBPtr*)wglGetProcAddress("wglCreateContextAttribsARB");
		wglChoosePixelFormatARB = (wglChoosePixelFormatARBPtr*)wglGetProcAddress("wglChoosePixelFormatARB");
		wglSwapIntervalEXT = (wglSwapIntervalEXTPtr*)wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT = (wglGetSwapIntervalEXTPtr*)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglMakeCurrent(DummyDC, 0);
		wglDeleteContext(DummyOpenGLContext);
		ReleaseDC(DummyWindowHandle, DummyDC);
		DestroyWindow(DummyWindowHandle);
		i32 pixelFormatAttribs[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0
		};
		
		i32 pixelFormat = 0;
		u32 numberOfFormats = 0;
		wglChoosePixelFormatARB(MainDeviceContext, pixelFormatAttribs, 0, 1, &pixelFormat, &numberOfFormats);
		Assert(numberOfFormats != 0);
		PIXELFORMATDESCRIPTOR DesiredDescriptor = {};
		DescribePixelFormat(MainDeviceContext, pixelFormat, sizeof(DesiredDescriptor), &DesiredDescriptor);
		if (!SetPixelFormat(MainDeviceContext, pixelFormat, &DesiredDescriptor))
		{
			Assert(false);
		}
		i32 openGLVersion[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, major_version,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor_version,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
		OpenGLRenderContext = wglCreateContextAttribsARB(MainDeviceContext, 0, openGLVersion);
		Assert(OpenGLRenderContext != NULL);
		b32 desiredContextResult = wglMakeCurrent(MainDeviceContext, OpenGLRenderContext);
		Assert(desiredContextResult == TRUE);
	}
	else
	{
		Assert(false);
	}
}
#endif
/*
* This function initializes XAudio2. This library requires a number of steps described by the Microsoft documentation
* 1. Initialize the COM Interface. This API relies heavily on COM and can't be used without it.
* 2. Load the XAudio2 dll based on the available version. XAudio2 is readily available on all windows installations and doesn't need redistribution
* 3. Retrieve an XAudio2 interface object
* 4. Create a mastering voice, which is a representation of the audio hardware
*/
internal void Win32InitXAudio2(Win32SoundOutput* Win32Sound)
{
	//TODO: Switch to playing a sine wave
	HRESULT debugResult = 0;
	debugResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	Assert(SUCCEEDED(debugResult));
	HMODULE XAudio2Library = LoadLibraryA("XAUDIO2_9.DLL");
		if (!XAudio2Library)
		{
			XAudio2Library = LoadLibraryA("XAUDIO2_8.DLL");
		}
		if (!XAudio2Library)
		{
			XAudio2Library = LoadLibraryA("XAUDIO2_7.DLL");
		}
        if(!XAudio2Library)
        {
            MessageBoxA(NULL, "Couldn't find supported version of XAudio2. Audio support disabled!", "Warning", MB_OK | MB_ICONWARNING);
            return;
        }
		debugResult = XAudio2Create(&Win32Sound->XAudioObject, 0, XAUDIO2_DEFAULT_PROCESSOR);
		Assert(SUCCEEDED(debugResult));
		debugResult = Win32Sound->XAudioObject->CreateMasteringVoice(&Win32Sound->Win32MasterVoice);
		Assert(SUCCEEDED(debugResult));
		Win32Sound->NumberOfChannels = 2; 
		Win32Sound->SamplesPerSecond = 44100;
		Win32Sound->BitsPerSample = 16;
		Win32Sound->BytesPerSample = 16 / 8;
		Win32Sound->SoundBufferSize = ((u64)Win32Sound->SamplesPerSecond * (u64)Win32Sound->BytesPerSample);
		WAVEFORMATEX waveFormat = {};
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = Win32Sound->NumberOfChannels;
		waveFormat.nSamplesPerSec = Win32Sound->SamplesPerSecond;
		waveFormat.wBitsPerSample = Win32Sound->BitsPerSample;
		waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		debugResult = Win32Sound->XAudioObject->CreateSourceVoice(&Win32Sound->Win32SourceVoice, &waveFormat);
		Assert(SUCCEEDED(debugResult));
}

/*
Sets up the XAUDIO2 sound buffer and submits it to the source  voice
@TODO: Currently, the audio we use is a simple wave that requires a single source voice, later on when we build the audio mixer, this will change
*/
internal void Win32SubmitAudio(Win32SoundOutput* Win32Sound)
{
    HRESULT debugResult = 0;
    Win32Sound->SoundBuffer = {};
    Win32Sound->SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;
    Win32Sound->SoundBuffer.AudioBytes = TruncateU64(Win32Sound->SoundBufferSize);
    Win32Sound->SoundBuffer.pAudioData = Win32Sound->SoundData;
    Win32Sound->SoundBuffer.PlayBegin = 0;
    Win32Sound->SoundBuffer.PlayLength = 0;
    Win32Sound->SoundBuffer.LoopBegin = 0;
    Win32Sound->SoundBuffer.LoopLength = 0;
    Win32Sound->SoundBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    Win32Sound->SoundBuffer.pContext = NULL;
    debugResult = Win32Sound->Win32SourceVoice->SubmitSourceBuffer(&Win32Sound->SoundBuffer);       
    Assert(SUCCEEDED(debugResult));
}

/*
Initialize XInput
Grab the dll from disk(ships with Windows)
If there is no version, a warning is displayed
Import 2 functions needed to use the API
*/
internal void Win32InitXInput()
{
	
	HMODULE XInputLibrary = LoadLibraryA("Xinput1_4.dll");
	if(!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("Xinput9_1_0.dll");
	}
	if(!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("Xinput1_3.dll");
	}
	if(!XInputLibrary)
	{
		MessageBoxA(NULL, "Could not find a supported version of XInput! Controller support disabled!", "Warning", MB_OK | MB_ICONWARNING);
	}
	XInputGetState = (xinput_get_state_ptr*)GetProcAddress(XInputLibrary, "XInputGetState");
	XInputSetState = (xinput_set_state_ptr*)GetProcAddress(XInputLibrary, "XInputSetState");
}

//Process digital buttons for XInput and Create input state for the game layer
/*
@Param(DWORD ButtonState) Bitfield for gamepad buttons
@Param(GameButtonState* OldState) The state of the controller in the last poll
@Param(GameButtonState* NewState) The state of the controller in the new poll
@Param(DWORD ButtonBit) bit to be masked in XINPUT_GAMEPAD structure
If the button bit was high, then the button state(Ended press) is true
If the Old controller press state not equal to the new controller press state, then a transition happended
*/
internal void Win32ProcessDigitalButton(DWORD ButtonState, GameButtonState* OldState, GameButtonState* NewState, DWORD ButtonBit)
{
	NewState->EndedPress = ((ButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedPress != NewState->EndedPress) ? 1 : 0;
}

internal void Win32ProcessKeyboardButton(GameButtonState* NewState, b32 KeyState)
{
	Assert(NewState->EndedPress != KeyState);
	NewState->EndedPress = KeyState;
	++NewState->HalfTransitionCount;
}

internal f32 Win32ProcessStickValue(SHORT value, SHORT deadzone)
{
	f32 result = 0;

	if(value < -deadzone)
	{
		result = (f32)((value + deadzone) / (32768.0f - deadzone));
	}
	else if(value > deadzone)
	{
		result = (f32)((value - deadzone) / (32767.0f - deadzone));
	}

	return result;
}



#ifdef SAND_INTERNAL

DEBUG_PLATFORM_FREE_FILE_MEMORY(DebugPlatformFreeFileMemory)
{
	if(FreeableMemory)
	{
		VirtualFree(FreeableMemory, 0, MEM_RELEASE);
	}
	else
	{
		//Log
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFile)
{
	DebugPlatformReadFileResult Result = {};
	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);

	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			u32 FileSize32 = TruncateU64(FileSize.QuadPart);
			Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(Result.Content)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
				{
					Result.ContentSize = FileSize32;
				}
				else
				{
					//ReadFile Failed
					DebugPlatformFreeFileMemory(Result.Content);
					Result.Content = 0;
				}
			}
			else
			{
				//File allocation failure
			}
		}
		else
		{
			//Couldn't get file size
		}

		CloseHandle(FileHandle);
	}
	else
	{
		//Couldn't Create file handle
	}

	return Result;
}



DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DebugPlatformWriteEntireFile)
{
	b32 Result = false;

	HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if(WriteFile(FileHandle, WriteableMemory, MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
		}
		else
		{
			//Couldn't write stuff
		}
		CloseHandle(FileHandle);
	}
	else
	{
		//File handle error
	}
	return Result;
}
#endif

internal void Win32ProcessMessages(GameController* KeyboardController)
{
			//Input polling
			MSG Message;
       		while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
			{
				switch(Message.message)
				{
					case WM_QUIT:
					{
						Win32IsRunning = false;
					}break;

					case WM_SYSKEYDOWN:
					case WM_SYSKEYUP:
					case WM_KEYDOWN:
					case WM_KEYUP:
					{
						/* As per the Win32 documentation:
						 * Key codes are stored in the wParam of a MSG 
						 * The key states (is key down, was key down, was alt key down)
						 * are stored in a bitmask, which is available in the documentation.
						 */
						Win32KeyCode = (u32)Message.wParam;
						Win32IsKeyDown = ((u32)Message.lParam & (1 << 31)) == 0;
						Win32WasKeyDown = ((u32)Message.lParam & (1 << 30)) != 0;
						Win32WasAltKeyDown = ((u32)Message.lParam & (1 << 29)) != 0;

						if (Win32WasKeyDown != Win32IsKeyDown)
						{
							switch (Win32KeyCode) 
							{
								case VK_F4:
								{
									if (Win32WasAltKeyDown)
									{
										Win32IsRunning = false;
									}
								}break;

								case 'W':
								{
									Win32ProcessKeyboardButton(&KeyboardController->DPadUp, Win32IsKeyDown);
								}break;

								case 'A':
								{
									Win32ProcessKeyboardButton(&KeyboardController->DPadLeft, Win32IsKeyDown);
								}break;

								case 'S':
								{
									Win32ProcessKeyboardButton(&KeyboardController->DPadDown, Win32IsKeyDown);
								}break;

								case 'D':
								{
									Win32ProcessKeyboardButton(&KeyboardController->DPadRight, Win32IsKeyDown);
								}break;
							}
						}
					}break;

					default:
					{
						TranslateMessage(&Message);
						DispatchMessageA(&Message);
					}break;
				}

			}
}

/*
Query the clock for the elapsed time
*/
internal LARGE_INTEGER Win32GetWallClock()
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return Result;
}

/*
Get seconds elapsed in a frame
*/
internal f64 Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	f64 ElapsedTime = ((f64)End.QuadPart - (f64)Start.QuadPart);
	f64 Result = (f64)(ElapsedTime / (f64)PerfFrequency.QuadPart);
	return Result;
}

struct Win32GameCode
{
	HMODULE GameLibrary;
	game_generate_audio* GameGenerateAudio;
	game_update*		 GameUpdate;
	b32 IsDLLValid;
};

internal Win32GameCode Win32LoadGameDLL()
{
	Win32GameCode Result = {};
	Result.GameLibrary = LoadLibraryA("Game.dll");

	CopyFile("Game.dll", "Game_temp.dll", FALSE);
	if(Result.GameLibrary)
	{
		Result.GameGenerateAudio = (game_generate_audio*)GetProcAddress(Result.GameLibrary, "GameGenerateAudio");
		Result.GameUpdate = (game_update*)GetProcAddress(Result.GameLibrary, "GameUpdate");

		Result.IsDLLValid = (Result.GameGenerateAudio && Result.GameUpdate);
	}
	else
	{
		//Failed to load dll
	}

	if(!Result.IsDLLValid)
	{
		Result.GameGenerateAudio = GameGenerateAudioStub;
		Result.GameUpdate = GameUpdateStub;
	}

	return Result;
}

internal void Win32UnloadGameDLL(Win32GameCode* GameCode)
{
	if(GameCode->GameLibrary)
	{
		FreeLibrary(GameCode->GameLibrary);
		GameCode->GameLibrary = 0;
	}

	GameCode->IsDLLValid = false;
	GameCode->GameUpdate = GameUpdateStub;
	GameCode->GameGenerateAudio = GameGenerateAudioStub;
}

internal void Render(i32 BlueOffset, i32 GreenOffset)
{
	i32 Width = BitmapWidth;

	BytesPerPixel = 4;
	i32 Pitch = Width * BytesPerPixel;

	u8* Row = (u8*)BitmapMemory;

	for(i32 Y = 0; Y < BitmapHeight; ++Y)
	{
		u32* Pixel = (u32*)Row;
		for(i32 X = 0; X < BitmapWidth; ++X)
		{
			u8 Blue = (u8)(X + BlueOffset);
			u8 Green = (u8)(Y + GreenOffset);

			*Pixel++ = ((Green << 8) | Blue); 
		}
		Row += Pitch;
	}
}

/*
Create or resize device-independent bitmap
*/
internal void Win32ResizeDIBSection(i32 width, i32 height)
{
	if(BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = width;
	BitmapHeight = height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	BytesPerPixel = 4;
	i32 BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;

	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

//update device-independent bitmap
internal void Win32UpdateWindow(HDC DeviceContext, RECT* ClientRect)
{
	i32 ClientWidth = ClientRect->right - ClientRect->left;
    i32 ClientHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(DeviceContext,
                  0, 0, BitmapWidth, BitmapHeight,
                  0, 0, ClientWidth, ClientHeight,
                  BitmapMemory,
                  &BitmapInfo,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK WindowsCallBack(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
	LRESULT result = 0;

	switch (msg)
	{	
	case WM_SIZE:
	{
        RECT ClientRect;
        GetClientRect(handle, &ClientRect);
        int Width = ClientRect.right - ClientRect.left;
        int Height = ClientRect.bottom - ClientRect.top;
        Win32ResizeDIBSection(Width, Height);
	}break;

	case WM_PAINT:
	{
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(handle, &Paint);
            RECT ClientRect;
            GetClientRect(handle, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect);
            EndPaint(handle, &Paint);
	}break;	

	case WM_CLOSE:
	{
		Win32IsRunning = false;
	}break;

	case WM_DESTROY:
	{
		Win32IsRunning = false;
	}break;

	default:
	{
		result = DefWindowProcA(handle, msg, wParam, lParam);
	}break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	QueryPerformanceFrequency(&PerfFrequency);

	u32 DesiredSchedulerMS = 1;
	b32 IsSleepGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
	u32 MonitorRefreshRate = 60; //Find a way to query this on the platform
	u32 GameRefreshRate = MonitorRefreshRate; //Try to target monitor refresh rate (30 frames at minimum)
	f32 TargetSecondsPerFrame = 1.0f / GameRefreshRate;


	Win32InitXInput();
    
	Win32SoundOutput Win32Sound = {};
	Win32InitXAudio2(&Win32Sound);


	WNDCLASSEXA Window = {};
	Window.hInstance = hInstance;
	Window.cbSize = sizeof(Window);
	Window.style = CS_HREDRAW | CS_VREDRAW;
	Window.lpfnWndProc = WindowsCallBack;
	Window.cbClsExtra = 0;
	Window.cbWndExtra = 0;
	Window.hIcon = LoadIconA(hInstance, IDI_APPLICATION);
	Window.hCursor = LoadCursorA(NULL, IDC_ARROW);
	Window.lpszMenuName = NULL;
	Window.lpszClassName = "MainWindow";
	Window.hIconSm = LoadIconA(Window.hInstance, IDI_APPLICATION);


	RegisterClassExA(&Window);

	HWND WindowHandle = CreateWindowExA(0, "MainWindow", "SandEngine: Platform Test App", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	ShowWindow(WindowHandle, nCmdShow);
	UpdateWindow(WindowHandle);

	if (WindowHandle)
	{
		#ifdef SAND_INTERNAL
		LPVOID BaseAddress = (LPVOID)TERABYTES(2);
		#else
		LPVOID BaseAddress = 0;
		#endif

		GameMemory Memory = {};
		Memory.DebugPlatformFreeFileMemory = DebugPlatformFreeFileMemory;
		Memory.DebugPlatformReadEntireFile = DebugPlatformReadEntireFile;
		Memory.DebugPlatformWriteEntireFile = DebugPlatformWriteEntireFile;
		Memory.PermanentStorageSize = MEGABYTES(64);
		Memory.TransientStorageSize = GIGABYTES(4);
		u64 TotalStorageSize = Memory.TransientStorageSize + Memory.PermanentStorageSize;

		Memory.PermanentStorage = VirtualAlloc(BaseAddress, TotalStorageSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		Memory.TransientStorage = ((u8*)Memory.PermanentStorage + Memory.PermanentStorageSize);
		Assert(Memory.PermanentStorage && Memory.TransientStorage);

		//Init audio
		Win32Sound.SoundData = (u8*)VirtualAlloc(0, Win32Sound.SoundBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		Assert(Win32Sound.SoundData);

		GameSoundOutput GameAudio = {};
		GameAudio.BitsPerSample    = Win32Sound.BitsPerSample;
		GameAudio.BytesPerSample   = Win32Sound.BytesPerSample;
		GameAudio.NumberOfChannels = Win32Sound.NumberOfChannels;
		GameAudio.SamplesPerSecond = Win32Sound.SamplesPerSecond;
		GameAudio.SoundBufferSize  = Win32Sound.SoundBufferSize;
		GameAudio.SoundData 	   = Win32Sound.SoundData;
		GameAudio.IsBufferFilled   = false;
		
		Win32SubmitAudio(&Win32Sound);
        Win32Sound.Win32SourceVoice->Start();

		//Setup Input
		GameInput Input[2] = {};
		GameInput* OldInput = &Input[0];
		GameInput* NewInput = &Input[1];

#if 0
		//Init graphics
		Win32InitOpenGL(4, 6);
		wglSwapIntervalEXT(0);
#endif


		Win32GameCode GameCode = Win32LoadGameDLL();
		u32 GameLoadCounter = 0;

		i32 XOffset = 0;
		i32 YOffset = 0;

		i64 LastCycleCount = __rdtsc();
		LARGE_INTEGER LastCounter = Win32GetWallClock();
		Win32IsRunning = true;
		while (Win32IsRunning)
		{
			if(GameLoadCounter++ > 240)
			{
				Win32UnloadGameDLL(&GameCode);
				GameCode = Win32LoadGameDLL();
				GameLoadCounter = 0;
			}

			GameController* OldKeyboardController = GetController(OldInput, 0);
			GameController* NewKeyboardController = GetController(NewInput, 0);
			*NewKeyboardController = {};
			
			NewKeyboardController->IsConnected = true;

			for(i32 ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
			{
				NewKeyboardController->Buttons[ButtonIndex].EndedPress = OldKeyboardController->Buttons[ButtonIndex].EndedPress;
			}

			Win32ProcessMessages(NewKeyboardController);

			//Gamepad input
			u32 MaxControllerCount = XUSER_MAX_COUNT;
			if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
			{
				MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
			}

			XINPUT_STATE ControllerState;
			for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
			{
				DWORD GamePadControllerIndex = ControllerIndex + 1;
				GameController* NewController = GetController(NewInput, GamePadControllerIndex);
				GameController* OldController = GetController(OldInput, GamePadControllerIndex);

				if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
				{
					XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;
					NewController->IsConnected = true;
					
					//Digital input
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadUp,   		&NewController->DPadUp, XINPUT_GAMEPAD_DPAD_UP);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadDown, 		&NewController->DPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadLeft,    		&NewController->DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadRight,    		&NewController->DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);

					Win32ProcessDigitalButton(Pad->wButtons, &OldController->Start, 	  	&NewController->Start, XINPUT_GAMEPAD_START);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->Back,     		&NewController->Back, XINPUT_GAMEPAD_BACK);

                    //LS, RS (L3, R3)
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->LeftThumb,  	&NewController->LeftThumb, XINPUT_GAMEPAD_LEFT_THUMB);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->RightThumb, 	&NewController->RightThumb, XINPUT_GAMEPAD_LEFT_THUMB);

					//(L1, R1)
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->LeftShoulder,   &NewController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->RightShoulder,  &NewController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);

					//A, B, X, Y
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->A, 			 &NewController->A, XINPUT_GAMEPAD_A);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->B, 			 &NewController->B, XINPUT_GAMEPAD_B);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->X, 			 &NewController->X, XINPUT_GAMEPAD_X);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->Y, 			 &NewController->Y, XINPUT_GAMEPAD_Y);
					

					//#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
					//#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
				    //#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30 (Do we really need this)??
					NewController->IsAnalog = true;
					//Analog input
					//L2
					NewController->LeftTriggerAverage = Pad->bLeftTrigger / 255.0f;

					//R2
					NewController->RightTriggerAverage = Pad->bRightTrigger / 255.0f;

					//Left thumb x axis
					NewController->LeftStickAverageX = Win32ProcessStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

					//left thumb y axis
					NewController->LeftStickAverageY = Win32ProcessStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

					//right thumb x axis
					NewController->RightStickAverageX = Win32ProcessStickValue(Pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

					//right thumb y axis
					NewController->RightStickAverageY = Win32ProcessStickValue(Pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
					NewController->IsAnalog = false;
				}
				else
				{
					//Controller is not connected
					NewController->IsConnected = false;
				}
			}

			Render(XOffset, YOffset);

			HDC DeviceContext = GetDC(WindowHandle);
			RECT ClientRect;
			GetClientRect(WindowHandle, &ClientRect);
			Win32UpdateWindow(DeviceContext, &ClientRect);
			ReleaseDC(WindowHandle, DeviceContext);

			++XOffset;
			++YOffset;

			GameCode.GameUpdate(&Memory, NewInput, &GameAudio);

			//Swap input states to always get latest state
			GameInput* temp = NewInput;
			NewInput = OldInput;
			OldInput = temp;

			/*
			Performance Calculation
			TODO: Better way to log performance numbers
			*/

			LARGE_INTEGER WorkCounter = Win32GetWallClock();
			f64 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

			f64 SecondsElapsedForFrame = WorkSecondsElapsed;
			if(SecondsElapsedForFrame < TargetSecondsPerFrame)
			{
				if(IsSleepGranular)
				{
					DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));

					if(SleepMS > 0)
					{
						Sleep(SleepMS);
					}
				}

				while(SecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
				}
			}
			else
			{
				//Log
			}

			i64 EndCycleCount = __rdtsc();
			LARGE_INTEGER EndCounter = Win32GetWallClock();

			f64 MegaCyclesElapsed = ((f64)EndCycleCount - (f64)LastCycleCount) / (1000.0f * 1000.0f);
			f64 MilliSecondsElapsed = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
			f64 FramesPerSecond = 1.0f / Win32GetSecondsElapsed(LastCounter, EndCounter);

			char buffer[512];
			sprintf_s(buffer, sizeof(buffer), "ms/frame: %.2f	Mc/frame: %.2f	FPS: %.2f\n", MilliSecondsElapsed, MegaCyclesElapsed, FramesPerSecond);
			OutputDebugStringA(buffer);

			LastCounter = EndCounter;
			LastCycleCount = EndCycleCount;
		}
	}
	else
	{
		//Logging
		OutputDebugStringA("Failed to create Window!");
	}
	return 0;
}