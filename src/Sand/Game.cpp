#include "Game.h"

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

internal void DrawRectangle(GameBackBuffer* Buffer, f32 MinX, f32 MinY, f32 MaxX, f32 MaxY, 
                            f32 Red, f32 Green, f32 Blue)
{
	i32 IntegerMinX = Roundf32Toi32(MinX);
	i32 IntegerMinY = Roundf32Toi32(MinY);
	i32 IntegerMaxX = Roundf32Toi32(MaxX);
	i32 IntegerMaxY = Roundf32Toi32(MaxY);
	
	if(IntegerMinX < 0)
	{
		IntegerMinX = 0;
	}
    
	if(IntegerMinY < 0)
	{
		IntegerMinY = 0;
	}
    
	if(IntegerMaxX > Buffer->BitmapWidth)
	{
		IntegerMaxX = Buffer->BitmapWidth;
	}
    
	if(IntegerMaxY > Buffer->BitmapHeight)
	{
		IntegerMaxY = Buffer->BitmapHeight;	
	}
    
	/*
	AA RR GG BB
	Alpha is 24 bits to the left (not used)
	Red is 16 bits to the left
	Green is 8 bits to the left
	Blue is 0 bits to the left
	*/
    u32 Color = ((Roundf32Tou32(Red * 255.0f) << 16) |
                 (Roundf32Tou32(Green * 255.0f) << 8) |
                 (Roundf32Tou32(Blue * 255.0f) << 0));
    
	u8* Row = ((u8*)Buffer->BitmapMemory) +
    (IntegerMinY * Buffer->Pitch) +
    (IntegerMinX * Buffer->BytesPerPixel);
    
	for(i32 Y = IntegerMinY;
        Y < IntegerMaxY;
        ++Y)
	{
		u32* Pixel = (u32*)Row;
		for(i32 X = IntegerMinX;
            X < IntegerMaxX;
            ++X)
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

internal b32 IsPointEmpty(World *world, f32 PointX, f32 PointY)
{
	b32 Result = false;
	i32 PlayerTileX = Truncatef32Toi32(((PointX - world->UpperLeftX) / world->TileWidth));
	i32 PlayerTileY = Truncatef32Toi32(((PointY - world->UpperLeftY) / world->TileHeight));
    
	if((PlayerTileX >= 0) && (PlayerTileX <= world->WidthCount) &&
	   (PlayerTileY >= 0) && (PlayerTileY <= world->HeightCount))
	{
		Result = (world->TileMaps->Data[(PlayerTileY * world->WidthCount) + PlayerTileX] == 0);
	}
    
	return Result;
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
        
		//State
		State->PlayerX = 150;
		State->PlayerY = 150;
        
		Memory->IsInitialized = true;
	}
    
#define TILE_WIDTH_COUNT 16
#define TILE_HEIGHT_COUNT 9
    
	// 9 rows, 16 columns
	u32 TileMap00[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
	};
    
    u32 TileMap01[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
	};
    
    u32 TileMap11[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
	};
    
    u32 TileMap10[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
	};
    
    
#define UPPER_LEFT_X  10
#define UPPER_LEFT_Y 10
#define TILE_WIDTH  60
#define TILE_HEIGHT  60
    
	TileMap TileMaps[2][2] = {};
    
    TileMaps[0][0].Data = (u32*)TileMap00;
    TileMaps[0][1].Data = (u32*)TileMap01;
    TileMaps[1][1].Data = (u32*)TileMap11;
    TileMaps[1][0].Data = (u32*)TileMap10;
    
    World world = {};
	world.HeightCount = TILE_HEIGHT_COUNT;
	world.WidthCount = TILE_WIDTH_COUNT;
	world.TileWidth =  TILE_WIDTH;
	world.TileHeight =  TILE_HEIGHT;
	world.UpperLeftX = UPPER_LEFT_X;
	world.UpperLeftY = UPPER_LEFT_Y;
	world.TileMaps = (TileMap*)TileMaps;
    
	f32 PlayerR = 0.3f;
	f32 PlayerG = 0.5f;
	f32 PlayerB = 0.2f;
	f32 PlayerLeft = State->PlayerX;  
	f32 PlayerTop = State->PlayerY;  
	f32 PlayerWidth = 0.25f * TILE_WIDTH;
	f32 PlayerHeight = TILE_HEIGHT;
    
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
			f32 dPlayerX = 0.0f;
			f32 dPlayerY = 0.0f;
			if(Controller0->DPadUp.EndedPress)
			{
				dPlayerY = -1.0f;
			}
            
			if(Controller0->DPadDown.EndedPress)
			{
				dPlayerY = 1.0f;
			}
            
			if(Controller0->DPadLeft.EndedPress)
			{
				dPlayerX = -1.0f;
			}
            
			if(Controller0->DPadRight.EndedPress)
			{
				dPlayerX = 1.0f;
			}
			dPlayerX *= 64.0f;
			dPlayerY *= 64.0f;
            
			f32 NewPlayerX = State->PlayerX + Input->TargetSecondsPerFrame*dPlayerX;
			f32 NewPlayerY = State->PlayerY + Input->TargetSecondsPerFrame*dPlayerY;
            
			if(IsPointEmpty(&world, NewPlayerX, NewPlayerY + PlayerHeight) &&
               IsPointEmpty(&world, NewPlayerX + PlayerWidth, NewPlayerY + PlayerHeight))
			{
				State->PlayerX = NewPlayerX;
				State->PlayerY = NewPlayerY;
			}
		}
	}
    
	//Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}
    
	//Render
    DrawRectangle(BackBuffer, 0.0f, 0.0f, (f32)BackBuffer->BitmapWidth, (f32)BackBuffer->BitmapHeight,
                  0.0f, 0.0f, 0.0f);
    
    
    
    for(i32 Row = 0;
        Row < 9;
        ++Row)
    {
        for(i32 Column = 0;
            Column < 17;
            ++Column)
        {
            u32 TileID = TileMap00[Row][Column];
            f32 Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 1.0f;
            }
            
            f32 MinX = UPPER_LEFT_X + ((f32)Column)*TILE_WIDTH;
            f32 MinY = UPPER_LEFT_Y + ((f32)Row)*TILE_HEIGHT;
            f32 MaxX = MinX + TILE_WIDTH;
            f32 MaxY = MinY + TILE_HEIGHT;
            DrawRectangle(BackBuffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    
	DrawRectangle(BackBuffer, PlayerLeft,
                  PlayerTop, PlayerLeft + PlayerWidth, PlayerTop + PlayerHeight,
                  PlayerR, PlayerG, PlayerB);
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