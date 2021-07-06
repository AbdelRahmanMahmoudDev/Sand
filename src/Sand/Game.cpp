#include "Game.h"

#include "Sand_Math.h"

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

internal TileMap * GetTileMap(World* world, i32 TileMapX, i32 TileMapY)
{
	TileMap *tileMap = 0;

	if((TileMapX >= 0) && (TileMapX < world->TileMapCountX) &&
	   (TileMapY >= 0) && (TileMapY < world->TileMapCountY))
	{
		  tileMap = &world->TileMaps[TileMapY * world->TileMapCountX + TileMapX]; 
	}

	return tileMap;
}

internal i32 GetTile(World* world, TileMap* tile_map, i32 TileX, i32 TileY)
{
	Assert(tile_map);
	Assert((TileX >= 0) && (TileX < world->WidthCount) &&
	       (TileY >= 0) && (TileY < world->HeightCount));

	u32 tile_value = tile_map->Data[(TileY * world->WidthCount) + TileX];

	return tile_value;
}

internal void RecanonicalizeCoord(World *world, i32 tile_count, i32 *tile_map_pos, i32 *tile_pos, f32 *tile_relative_pos)
{
	i32 offset = Floorf32Toi32(*tile_relative_pos / world->TileWidthInMeters);
	*tile_pos += offset;
	*tile_relative_pos -= offset * world->TileWidthInMeters;

	Assert(*tile_relative_pos >= 0);

	Assert(*tile_relative_pos <= world->TileWidthInMeters);

	if(*tile_pos < 0)
	{
		*tile_pos = tile_count + *tile_pos;
		--*tile_map_pos;
	}

	if(*tile_pos >= tile_count)
	{
		*tile_pos = *tile_pos - tile_count;
		++*tile_map_pos;
	}
}

internal CanonicalPosition RecanonicalizePosition(World *world, CanonicalPosition position)
{
	CanonicalPosition Result = position;

	RecanonicalizeCoord(world, world->WidthCount, &Result.TileMapX, &Result.TileX, &Result.TileRelativeX);
	RecanonicalizeCoord(world, world->HeightCount, &Result.TileMapY, &Result.TileY, &Result.TileRelativeY);

	return Result;
}

internal b32 IsTileMapPointEmpty(World *world, TileMap *tile_map, i32 PointX, i32 PointY)
{
	b32 Result = false;
    
	if((PointX >= 0) && (PointX <= world->WidthCount) &&
	   (PointY >= 0) && (PointY <= world->HeightCount))
	{
		u32 TileMapValue = GetTile(world, tile_map, PointX, PointY); 
		Result = ( TileMapValue == 0);
	}
    
	return Result;
}

internal b32 IsWorldPointEmpty(World *world, CanonicalPosition Position)
{
	b32 Result = false;

	TileMap *tile_map = GetTileMap(world, Position.TileMapX, Position.TileMapY);
	Result = IsTileMapPointEmpty(world, tile_map, Position.TileX, Position.TileY);

	return Result;
}

extern "C" GAME_UPDATE(GameUpdate)
{
#define TILE_WIDTH_COUNT 16
#define TILE_HEIGHT_COUNT 9
#define UPPER_LEFT_X  0
#define UPPER_LEFT_Y 0
#define TILE_WIDTH  60
#define TILE_HEIGHT  60
	    
	//Memory
	Assert(sizeof(GameState) <= Memory->PermanentStorageSize);
	GameState* State = (GameState*)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		//Placeholder thread thing
		ThreadContext thread = {};
        
		//State
		State->PlayerPos.TileMapX = 0;
		State->PlayerPos.TileMapY = 0;
		State->PlayerPos.TileX = 3;
		State->PlayerPos.TileY = 3;
		State->PlayerPos.TileRelativeX = 5.0f;
		State->PlayerPos.TileRelativeY = 5.0f;

		Memory->IsInitialized = true;
	}

	// 9 rows, 16 columns
	u32 TileMap00[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
		{1, 1, 1, 1,  1, 1, 1,   1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0,   0, 0, 0, 0,  0, 1, 0, 0, 1},
        {1, 1, 0, 0,  0, 0, 0,   1, 0, 0, 0,  0, 0, 1, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,   1, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0,   1, 0, 0, 0,  0, 0, 0, 0, 0},
        {1, 1, 0, 0,  0, 1, 0,   1, 0, 0, 0,  0, 1, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0,   1, 0, 0, 0,  1, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0,   0, 0, 0, 0,  0, 1, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1,   0, 1, 1, 1,  1, 1, 1, 1, 1},
	};
    
    u32 TileMap01[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
        {1, 1, 1, 1,  1, 1, 1,   0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1,   1, 1, 1, 1,  1, 1, 1, 1, 1},
	};
    
    u32 TileMap11[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
        {1, 1, 1, 1,  1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
	};
    
    u32 TileMap10[TILE_HEIGHT_COUNT][TILE_WIDTH_COUNT] = 
	{
        {1, 1, 1, 1,  1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
	};
    
	TileMap TileMaps[2][2];
    
	TileMaps[0][0].Data = (u32*)TileMap00;
    TileMaps[1][0].Data = (u32*)TileMap01;
    TileMaps[0][1].Data = (u32*)TileMap10;
    TileMaps[1][1].Data = (u32*)TileMap11;

	World world = {};
	world.TileMapCountX = 2;
	world.TileMapCountY = 2;
	world.WidthCount = TILE_WIDTH_COUNT;
	world.HeightCount = TILE_HEIGHT_COUNT;

	world.TileWidthInMeters = 1.4f;
	world.TileWidthInPixels = 60;
	world.MetersToPixels = (f32)world.TileWidthInPixels / (f32)world.TileWidthInMeters;

	world.UpperLeftX = UPPER_LEFT_X;
	world.UpperLeftY = UPPER_LEFT_Y;
	world.TileWidth =  TILE_WIDTH;
	world.TileHeight =  TILE_HEIGHT;

	world.TileMaps = (TileMap*)TileMaps;

	f32 PlayerR = 0.3f;
	f32 PlayerG = 0.5f;
	f32 PlayerB = 0.2f;
	f32 PlayerHeight = 1.4f;
	f32 PlayerWidth = 0.75f * PlayerHeight;
    
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
			dPlayerX *= 2.0f;
			dPlayerY *= 2.0f;
            
			CanonicalPosition NewPlayerPos = State->PlayerPos;
			NewPlayerPos.TileRelativeX += Input->TargetSecondsPerFrame*dPlayerX;
			NewPlayerPos.TileRelativeY += Input->TargetSecondsPerFrame*dPlayerY;
			NewPlayerPos = RecanonicalizePosition(&world, NewPlayerPos);
            
			CanonicalPosition PlayerLeft = NewPlayerPos;
			PlayerLeft.TileRelativeX -= 0.5f * PlayerWidth;
			PlayerLeft = RecanonicalizePosition(&world, PlayerLeft);

			CanonicalPosition PlayerRight = NewPlayerPos;
			PlayerRight.TileRelativeX += 0.5f * PlayerWidth;
			PlayerRight = RecanonicalizePosition(&world, PlayerRight);
#if 1
			if(IsWorldPointEmpty(&world, NewPlayerPos) &&
			   IsWorldPointEmpty(&world, PlayerLeft) &&
			   IsWorldPointEmpty(&world, PlayerRight))
			{
				State->PlayerPos = NewPlayerPos;
			}
#else
			State->PlayerPos = NewPlayerPos;
#endif
		}
	}
    
	//Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}
    
	TileMap *tile_map = GetTileMap(&world, State->PlayerPos.TileMapX, State->PlayerPos.TileMapY);
	Assert(tile_map);

	//Render
    DrawRectangle(BackBuffer, 0.0f, 0.0f, (f32)BackBuffer->BitmapWidth, (f32)BackBuffer->BitmapHeight,
                  1.0f, 0.0f, 0.0f);
    
    
    
    for(i32 Row = 0;
        Row < TILE_HEIGHT_COUNT;
        ++Row)
    {
        for(i32 Column = 0;
            Column < TILE_WIDTH_COUNT;
            ++Column)
        {
            u32 TileID = GetTile(&world, tile_map, Column, Row);
            f32 Gray = 0.5f;

            if(TileID == 1)
            {
                Gray = 1.0f;
            }
            
			#if 1
			if(Column == State->PlayerPos.TileX && Row == State->PlayerPos.TileY)
			{
				Gray = 0.0f;
			}
			#endif

            f32 MinX = world.UpperLeftX + ((f32)Column)*world.TileWidthInPixels;
            f32 MinY = world.UpperLeftY + ((f32)Row)*world.TileWidthInPixels;
            f32 MaxX = MinX + world.TileWidthInPixels;
            f32 MaxY = MinY + world.TileWidthInPixels;
            DrawRectangle(BackBuffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    

	f32 PlayerLeft = world.UpperLeftX + world.TileWidthInPixels*State->PlayerPos.TileX +
	 world.MetersToPixels*State->PlayerPos.TileRelativeX - 0.5f*world.MetersToPixels*PlayerWidth;

	 f32 PlayerTop = world.UpperLeftY + world.TileWidthInPixels*State->PlayerPos.TileY +
	 world.MetersToPixels*State->PlayerPos.TileRelativeY - 0.5f*world.MetersToPixels*PlayerHeight;

	DrawRectangle(BackBuffer,
				  PlayerLeft, PlayerTop,
				  PlayerLeft + world.MetersToPixels*PlayerHeight, PlayerTop + world.MetersToPixels*PlayerHeight,
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