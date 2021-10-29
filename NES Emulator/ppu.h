#ifndef _PPU_H_
#define _PPU_H_

#include "types.h"
#include "SDL.h"

struct Bus;

struct Pixel
{
	Byte r;
	Byte g;
	Byte b;
};

struct FIFO16
{
	union 
	{
		struct
		{
			Byte lo;
			Byte hi;
		};

		Word data;
	};
};

union OAMEntry
{
	struct
	{
		Byte y;
		Byte tile;
		Byte attr;
		Byte x;
	};

	DWord raw;
};


struct PPU
{
	////////////////////////////////////////
	///            REGISTERS             ///
	////////////////////////////////////////
	union
	{
		struct 
		{
			Byte nametable : 2;
			Byte increment : 1;
			Byte spriteTile : 1;
			Byte bgTile : 1;
			Byte spriteHeight : 1;
			Byte master : 1;
			Byte nmiEnable : 1;
		};

		Byte raw;

	} ppuCtrl;

	union
	{
		struct
		{
			Byte greyscale : 1;
			Byte bgLeftColumn : 1;
			Byte spriteLeftColumn : 1;
			Byte bgEnable : 1;
			Byte spriteEnable : 1;
			Byte colorEmphasis : 3;
		};

		Byte raw;

	} ppuMask;

	union
	{
		struct
		{
			Byte padding : 5;
			Byte overflow : 1;
			Byte spriteZeroHit : 1;
			Byte vBlank : 1;
		};

		Byte raw;

	} ppuStatus;

	Byte oamaddr;
	Byte oamdata;

	////////////////////////////////////////
	///         PSEUDO REGISTERS         ///
	////////////////////////////////////////
	Byte scrollX, scrollY;
	Byte scrollWriteTarget;

	union
	{
		struct
		{
			Byte lo;
			Byte hi;
		};

		Word raw;
	} ppuAddress;
	Byte ppuAddressWriteTarget;

	Byte oamdma;

	////////////////////////////////////////
	///              VRAM                ///
	////////////////////////////////////////

	Byte* nameTables[2];
	SDL_Texture* nameTableTextures[2];
	SDL_Texture* renderedNameTableTextures[2];
	Byte mirroring;

	Byte* paletteIndexes;


	////////////////////////////////////////
	///         PHASE TRACKERS           ///
	////////////////////////////////////////

	enum 
	{
		Visible,
		PostRender,
		VBlank,
		PreRender,
		VerticalPhaseSize
	} verticalPhase;

	enum
	{
		Idle,
		Fetching,
		SpriteFetching,
		NextLineFetching,
		Unknown,
		HorizontalPhaseSize
	} horizontalPhase;

	Byte remainingCycles;

	enum
	{
		Nametable,
		Attribute,
		PatternLow,
		PatternHigh,
		FetchingPhaseSize
	} fetchingPhase;

	////////////////////////////////////////
	///            TILE DATA             ///
	////////////////////////////////////////

	struct
	{
		Byte x;
		Byte y;
	} nametablePos;
	Byte tilePosY;

	struct
	{
		Byte nametable;
		Byte attribute;
		
		union
		{
			struct {
				Byte lo;
				Byte hi;
			};

			Word raw;

		} tile;
	} tileData;

	struct FIFO16 loPatternFIFO;
	struct FIFO16 hiPatternFIFO;

	union OAMEntry* oam;

	Word x, y;

	struct Pixel* pixels;
	SDL_Texture* screen;
	struct Bus* bus;
};

struct PPU* createPPU(struct Bus* parent);
void destroyPPU(struct PPU* ppu);

Byte ppuRead(struct PPU* ppu, Word addr);
void ppuWrite(struct PPU* ppu, Word addr, Byte val);

int tickPPU(struct PPU* ppu);

SDL_Texture* getNameTableTexture(struct PPU* ppu, int index);
SDL_Texture* getScreenTexture(struct PPU* ppu);
SDL_Texture* getRenderedNameTableTexture(struct PPU* ppu, int index);

#endif // _PPU_H_
