#ifndef _PPU_H_
#define _PPU_H_

#include "types.h"

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

	} ppuctrl;

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

	} ppumask;

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

	} ppustatus;

	Byte oamaddr;
	Byte oamdata;
	Byte ppuScroll;
	Byte ppuAddr;
	Byte ppuData;
	Byte oamdma;

	Byte* patternTables[2];
	Byte* nametables[4];
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

void tickPPU(struct PPU* ppu);

#endif // _PPU_H_