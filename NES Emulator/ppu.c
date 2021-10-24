#include "ppu.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "bus.h"
#include "cpu.h"
#include "cartridge.h"

struct PPU* createPPU(struct Bus* parent)
{
	struct PPU* ppu = (struct PPU*)malloc(sizeof(struct PPU));
	if (ppu == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU object.\n");
		exit(1);
	}

	ppu->bus = parent;



	ppu->patternTables[0] = (Byte*)malloc(0x1000 * 2);
	if (ppu->patternTables[0] == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU pattern tables.\n");
		exit(1);
	}
	memset(ppu->patternTables[0], 0, 0x1000 * 2);

	for (int i = 0; i < 2; i++)
	{
		ppu->patternTables[i] = ppu->patternTables[0] + ((size_t)0x1000 * i);
		ppu->patternTableTextures[i] = SDL_CreateTexture(parent->screen, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 64, 64);
	}



	ppu->nameTables[0] = (Byte*)malloc(0x0400 * 2);
	if (ppu->nameTables[0] == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU nameTables.\n");
		exit(1);
	}
	memset(ppu->nameTables[0], 0, 0x0400 * 2);

	for (int i = 0; i < 2; i++)
	{
		ppu->nameTables[i] = ppu->nameTables[0] + ((size_t)0x0400 * i);
		ppu->nameTableTextures[i] = SDL_CreateTexture(parent->screen, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 32, 32);
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


	ppu->mirroring = parent->cartridge->header.Flags6.mirror;

	ppu->ppuCtrl.raw = 0b00000000;
	ppu->ppuMask.raw = 0b00000000;
	ppu->ppuStatus.raw = 0b10100000;
	ppu->oamaddr = 0x00;

	ppu->scrollX = 0;
	ppu->scrollY = 0;
	ppu->scrollWriteTarget = 0;

	ppu->ppuAddress.raw = 0x00;
	ppu->ppuAddressWriteTarget = 1;

	ppu->x = 0;
	ppu->y = 0;

	return ppu;
}

void destroyPPU(struct PPU* ppu)
{
	free(ppu->oam);
	free(ppu->paletteIndexes);

	for (int i = 0; i < 2; i++)
		SDL_DestroyTexture(ppu->nameTableTextures[i]);
	free(ppu->nameTables[0]);

	for (int i = 0; i < 2; i++)
		SDL_DestroyTexture(ppu->patternTableTextures[i]);
	free(ppu->patternTables[0]);

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
	{
		if (ppu->ppuAddress.raw < 0x2000)
		{
			val = ppu->patternTables[0][ppu->ppuAddress.raw];
		}
		else if (0x2000 <= ppu->ppuAddress.raw && ppu->ppuAddress.raw < 0x3F00)
		{
			Word effectiveAddress = ppu->ppuAddress.raw;
			effectiveAddress &= ~(0x0400 + ppu->mirroring * 0x0400);
			if (effectiveAddress >= 0x2800)
				effectiveAddress -= 0x0400;

			val = ppu->nameTables[0][(effectiveAddress - 0x2000) & 0x0FFF];
		}
		else if (0x3F00 <= ppu->ppuAddress.raw && ppu->ppuAddress.raw < 0x4000)
		{
			val = ppu->paletteIndexes[(ppu->ppuAddress.raw - 0x3F00) & 0x1F];
		}
		else
		{
			fprintf(stderr, "PPU access violation at $%04X", ppu->ppuAddress.raw);
			exit(1);
		}
		ppu->ppuAddress.raw += ppu->ppuCtrl.increment;
	}
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
	{
		if (ppu->ppuAddress.raw < 0x2000)
		{
			ppu->patternTables[0][ppu->ppuAddress.raw] = val;
		}
		else if(0x2000 <= ppu->ppuAddress.raw && ppu->ppuAddress.raw < 0x3F00)
		{
			Word effectiveAddress = ppu->ppuAddress.raw;
			effectiveAddress &= ~(0x0400 + ppu->mirroring * 0x0400);
			if (effectiveAddress >= 0x2800)
				effectiveAddress -= 0x0400;

			ppu->nameTables[0][(effectiveAddress - 0x2000) & 0x0FFF] = val;
		}
		else if (0x3F00 <= ppu->ppuAddress.raw && ppu->ppuAddress.raw < 0x4000)
		{
			ppu->paletteIndexes[(ppu->ppuAddress.raw - 0x3F00) & 0x1F] = val;
		}
		else
		{
			fprintf(stderr, "PPU access violation at $%04X", ppu->ppuAddress.raw);
			exit(1);
		}
		ppu->ppuAddress.raw += (ppu->ppuCtrl.increment == 0 ? 1 : 32);
		break;
	}

	default:
		fprintf(stderr, "Write access violation at PPU register: $%04X", addr);
		exit(1);
	}
}

int tickPPU(struct PPU* ppu)
{
	// Do stuff
	if (ppu->x == 0 && ppu->y == 241)
		NMI(ppu->bus->cpu);

	// Increment counters
	ppu->x++;

	if (ppu->x == 341)
	{
		ppu->x = 0;
		ppu->y++;

		if (ppu->y == 261)
			ppu->y = 0;
	}

	return (ppu->x == 0 && ppu->y == 0);
}

SDL_Texture* getPatternTableTexture(struct PPU* ppu, int index)
{
	SDL_Texture* target = ppu->patternTableTextures[index];
	int pitch;
	void* pixels;
	SDL_LockTexture(target, NULL, &pixels, &pitch);
	SDL_memcpy(pixels, ppu->patternTables[index], 0x1000);
	SDL_UnlockTexture(target);
	return target;
}

SDL_Texture* getNameTableTexture(struct PPU* ppu, int index)
{
	SDL_Texture* target = ppu->nameTableTextures[index];
	int pitch;
	void* pixels;
	SDL_LockTexture(target, NULL, &pixels, &pitch);
	SDL_memcpy(pixels, ppu->nameTables[index], 0x0400);
	SDL_UnlockTexture(target);
	return target;
}