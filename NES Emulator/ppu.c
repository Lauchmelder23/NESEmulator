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
		ppu->renderedNameTableTextures[i] = SDL_CreateTexture(parent->screen, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	}



	ppu->paletteIndexes = (Byte*)malloc(0x20);
	if (ppu->paletteIndexes == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU palette RAM indexes.\n");
		exit(1);
	}



	ppu->oam = (union OAMEntry*)malloc(0x100);
	if (ppu->oam == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for PPU OAM.\n");
		exit(1);
	}

	ppu->pixels = (struct Pixel*)malloc(256 * 240 * sizeof(struct Pixel));
	ppu->screen = SDL_CreateTexture(parent->screen, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	if (ppu->screen == NULL)
	{
		fprintf(stderr, "Failed to create output screen texture\n");
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

	ppu->nametablePos.x = 0;
	ppu->nametablePos.y = 0;
	ppu->tilePosY = 0;

	ppu->verticalPhase = Visible;
	ppu->horizontalPhase = Idle;
	ppu->fetchingPhase = Nametable;
	ppu->remainingCycles = 1;

	ppu->x = 0;
	ppu->y = 0;

	return ppu;
}

void destroyPPU(struct PPU* ppu)
{
	free(ppu->oam);
	free(ppu->paletteIndexes);

	for (int i = 0; i < 2; i++)
	{
		SDL_DestroyTexture(ppu->renderedNameTableTextures[i]);
		SDL_DestroyTexture(ppu->nameTableTextures[i]);
	}
	free(ppu->nameTables[0]);

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

	case 0x2003:
		break;

	case 0x2005:
		break;

	case 0x2006:
		break;

	case 0x2007:
	{
		if (ppu->ppuAddress.raw < 0x2000)
		{
			val = readCartridgePPU(ppu->bus->cartridge, ppu->ppuAddress.raw);
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
		ppu->ppuAddress.raw += 1 + 31 * (ppu->ppuCtrl.increment == 1);
	}
		break;

	default:
		// fprintf(stderr, "Read access violation at PPU register $%04X", addr);
		// exit(1);
		break;
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
			writeCartridgePPU(ppu->bus->cartridge, ppu->ppuAddress.raw, val);
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
		ppu->ppuAddress.raw += 1 + 31 * (ppu->ppuCtrl.increment == 1);
		break;
	}

	default:
		//fprintf(stderr, "Write access violation at PPU register: $%04X", addr);
		//exit(1);
		break;
	}
}

int tickPPU(struct PPU* ppu)
{
	// Do stuff
	if(ppu->ppuStatus.vBlank)
		if (ppu->x == 1 && ppu->y == 241)
			NMI(ppu->bus->cpu);

	switch (ppu->verticalPhase)
	{
	case PreRender:
		break;

	case Visible:
	{
		// Fetching
		switch (ppu->horizontalPhase)
		{
		case Idle:
			break;

		case Fetching:
			ppu->remainingCycles--;
			if (ppu->remainingCycles == 0)
			{
				Byte tileY = ppu->y / 8;
				Byte tileX = (ppu->x - 1) / 8 + 2;

				switch (ppu->fetchingPhase)
				{
				case Nametable:
				{
					if (ppu->x != 1)
					{
						ppu->hiPatternFIFO.lo = ppu->tileData.tile.hi;
						ppu->loPatternFIFO.lo = ppu->tileData.tile.lo;
					}

					ppu->tileData.nametable = ppu->nameTables[ppu->ppuCtrl.nametable][(size_t)32 * tileY + tileX];
					break;
				}

				case Attribute:
				{
					ppu->tileData.attribute = ppu->nameTables[ppu->ppuCtrl.nametable][0x3C0 + ((tileY >> 2) * 8) + (tileX >> 2)];
					break;
				}

				case PatternLow:
				{
					ppu->tileData.tile.lo = readCartridgePPU(ppu->bus->cartridge, 0x1000 * ppu->ppuCtrl.bgTile + (ppu->tileData.nametable * 16) + (ppu->y % 8));
					break;
				}

				case PatternHigh:
				{
					ppu->tileData.tile.hi = readCartridgePPU(ppu->bus->cartridge, 0x1000 * ppu->ppuCtrl.bgTile + (ppu->tileData.nametable * 16) + (ppu->y % 8) + 8);
					break;
				}
				}

				ppu->remainingCycles = 2;
				ppu->fetchingPhase = (ppu->fetchingPhase + 1) % FetchingPhaseSize;
			}

			// Rendering (quick and dirty for now)
			Byte color = ((ppu->hiPatternFIFO.hi & 0x80) >> 6) | ((ppu->loPatternFIFO.hi & 0x80) >> 7);
			size_t index = (size_t)ppu->y * 256 + ppu->x - 1;
			// printf("index %d,%d -> %d\n", ppu->y, ppu->x - 1, index);
			ppu->pixels[index].r = 50 * color;
			ppu->pixels[index].g = 50 * color;
			ppu->pixels[index].b = 50 * color;

			ppu->loPatternFIFO.data <<= 1;
			ppu->hiPatternFIFO.data <<= 1;

			break;
		}
	} break;
	}

	// Increment counters
	ppu->x++;
	
	if (ppu->x == 341)
	{
		ppu->x = 0;
		ppu->y++;

		if (ppu->y == 262)
			ppu->y = 0;

		if (ppu->y == 261 || ppu->y == 0 || ppu->y == 240 || ppu->y == 241)
			ppu->verticalPhase = (ppu->verticalPhase + 1) % VerticalPhaseSize;
	}

	if (ppu->x == 0 || ppu->x == 1 || ppu->x == 257 || ppu->x == 321 || ppu->x == 337)
		ppu->horizontalPhase = (ppu->horizontalPhase + 1) % HorizontalPhaseSize;


	return (ppu->x == 0 && ppu->y == 0);
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

SDL_Texture* getScreenTexture(struct PPU* ppu)
{
	void* pixels;
	int stride;
	SDL_LockTexture(ppu->screen, NULL, &pixels, &stride);
	SDL_memcpy(pixels, ppu->pixels, 256 * 240 * sizeof(struct Pixel));
	SDL_UnlockTexture(ppu->screen);

	return ppu->screen;
}

SDL_Texture* getRenderedNameTableTexture(struct PPU* ppu, int index)
{
	SDL_Texture* target = ppu->renderedNameTableTextures[index];
	int pitch;
	struct Pixel* pixels;
	SDL_LockTexture(target, NULL, (void**)&pixels, &pitch);

	Byte patternTable = 0x1000 * ppu->ppuCtrl.bgTile;
	for (int y = 0; y < 30; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			Byte offset = ppu->nameTables[index][y * 32 + x];

			for (int row = 0; row < 8; row++)
			{
				Byte dataHi = readCartridgePPU(ppu->bus->cartridge, patternTable + offset * 16 + row + 8);
				Byte dataLo = readCartridgePPU(ppu->bus->cartridge, patternTable + offset * 16 + row);

				for (int bit = 0; bit < 8; bit++)
				{
					Byte color = (((dataHi << bit) & 0x80) >> 6) | (((dataLo << bit) & 0x80) >> 7);
					pixels[y * (256 * 8) + row * 256 + x * 8 + bit].r = 50 * color;
					pixels[y * (256 * 8) + row * 256 + x * 8 + bit].g = 50 * color;
					pixels[y * (256 * 8) + row * 256 + x * 8 + bit].b = 50 * color;
				}
			}
		}
	}

	SDL_UnlockTexture(target);
	return target;
}
