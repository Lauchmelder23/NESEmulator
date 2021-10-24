#ifndef _PPU_H_
#define _PPU_H_

#include "types.h"
#include "SDL.h"

struct Bus;

struct PPU
{
	// REGISTERS
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

	Byte* patternTables[2];
	SDL_Texture* patternTableTextures[2];

	Byte* nameTables[2];
	SDL_Texture* nameTableTextures[2];
	Byte mirroring;

	Byte* paletteIndexes;

	union
	{
		struct
		{
			Byte y;
			Byte tile;
			Byte attr;
			Byte x;
		};

		DWord raw;
	}* oam;

	Word x, y;

	struct Bus* bus;
};

struct PPU* createPPU(struct Bus* parent);
void destroyPPU(struct PPU* ppu);

Byte ppuRead(struct PPU* ppu, Word addr);
void ppuWrite(struct PPU* ppu, Word addr, Byte val);

int tickPPU(struct PPU* ppu);

SDL_Texture* getPatternTableTexture(struct PPU* ppu, int index);
SDL_Texture* getNameTableTexture(struct PPU* ppu, int index);

#endif // _PPU_H_