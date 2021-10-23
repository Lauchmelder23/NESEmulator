#include "ppu.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "bus.h"

struct PPU* createPPU(struct Bus* parent)
{
	struct PPU* ppu = (struct PPU*)malloc(sizeof(struct PPU));
	if (ppu == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU object.\n");
		exit(1);
	}

	ppu->bus = parent;

	for (int i = 0; i < 2; i++)
	{
		ppu->patternTables[i] = (Byte*)malloc(0x1000);
		if (ppu->patternTables[i] == NULL)
		{
			fprintf(stderr, "Failed to allocate memory for PPU pattern table #%d object.\n", i);
			exit(1);
		}

		memset(ppu->patternTables[i], 0, 0x1000);
	}

	for (int i = 0; i < 4; i++)
	{
		ppu->nametables[i] = (Byte*)malloc(0x0400);
		if (ppu->nametables[i] == NULL)
		{
			fprintf(stderr, "Failed to allocate memory for PPU nametable #%d object.\n", i);
			exit(1);
		}

		memset(ppu->nametables[i], 0, 0x0400);
	}

	ppu->paletteIndexes = (Byte*)malloc(0x20);
	if (ppu->paletteIndexes == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU palette RAM indexes.\n");
		exit(1);
	}

	ppu->oam = (Byte*)malloc(0x100);
	if (ppu->oam == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU OAM.\n");
		exit(1);
	}

	ppu->ppuCtrl.raw = 0b00000000;
	ppu->ppuMask.raw = 0b00000000;
	ppu->ppuStatus.raw = 0b10100000;
	ppu->oamaddr = 0x00;

	ppu->scrollX = 0;
	ppu->scrollY = 0;
	ppu->scrollWriteTarget = 0;

	ppu->ppuAddress.raw = 0x00;
	ppu->ppuAddressWriteTarget = 0;

	ppu->x = 0;
	ppu->y = 0;

	return ppu;
}

void destroyPPU(struct PPU* ppu)
{
	free(ppu->oam);
	free(ppu->paletteIndexes);
	free(ppu->nametables);
	free(ppu->patternTables);

	free(ppu);
}

Byte ppuRead(struct PPU* ppu, Word addr)
{
	Byte val = 0x00;
	switch (addr)
	{
	case 0x2000:
		break;

	case 0x2001:
		break;

	case 0x2002:
		val = ppu->ppuStatus.raw;
		break;

	case 0x2005:
		break;

	case 0x2006:
		break;

	case 0x2007:
		val = ppu->nametables[ppu->ppuAddress.raw & 0x2000][ppu->ppuAddress.raw & 0x1FFF];
		ppu->ppuAddress.raw += ppu->ppuCtrl.increment;
		break;

	default:
		fprintf(stderr, "Read access violation at PPU register $%04X", addr);
		exit(1);
	}

	return val;
}

void ppuWrite(struct PPU* ppu, Word addr, Byte val)
{
	switch (addr)
	{
	case 0x2000:
		ppu->ppuCtrl.raw = val;
		break;

	case 0x2001:
		ppu->ppuMask.raw = val;
		break;

	case 0x2002:
		break;

	case 0x2005:
		*((&ppu->scrollX) + ppu->scrollWriteTarget) = val;
		ppu->scrollWriteTarget = 1 - ppu->scrollWriteTarget;
		break;

	case 0x2006:
		*((&ppu->ppuAddress.lo) + ppu->ppuAddressWriteTarget) = val;
		ppu->ppuAddressWriteTarget = 1 - ppu->ppuAddressWriteTarget;
		break;

	case 0x2007:
		ppu->nametables[ppu->ppuAddress.raw & 0x2000][ppu->ppuAddress.raw & 0x1FFF] = val;
		ppu->ppuAddress.raw += ppu->ppuCtrl.increment;
		break;

	default:
		fprintf(stderr, "Write access violation at PPU register: $%04X", addr);
		exit(1);
	}
}

void tickPPU(struct PPU* ppu)
{
	ppu->x++;

	if (ppu->x == 341)
	{
		ppu->x = 0;
		ppu->y++;

		if (ppu->y == 261)
			ppu->y = 0;
	}
}
