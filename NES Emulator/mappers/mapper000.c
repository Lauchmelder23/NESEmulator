#include "mapper000.h"

#include <stdlib.h>
#include <stdio.h>

struct Mapper000* createMapper000(Byte prg_rom_size, Byte chr_rom_size, FILE* fp)
{
	struct Mapper000* mapper = (struct Mapper000*)malloc(sizeof(struct Mapper000));
	if (mapper == NULL)
	{
		fprintf(stderr, "Failed to create Mapper 000. Aborting\n");
		exit(-1);
	}

	mapper->prg_rom_size = prg_rom_size;
	mapper->prg_rom = (Byte*)calloc(prg_rom_size, 0x4000);
	if (mapper->prg_rom == NULL)
	{
		fprintf(stderr, "Failed to allocate cartridge PRG ROM. Aborting\n");
		exit(-1);
	}

	mapper->chr_rom_size = chr_rom_size;
	mapper->chr_rom = (Byte*)calloc(chr_rom_size, 0x2000);
	if (mapper->chr_rom == NULL)
	{
		fprintf(stderr, "Failed to allocate cartridge CHR ROM. Aborting\n");
		exit(-1);
	}

	fread(mapper->prg_rom, 0x4000, prg_rom_size, fp);
	fread(mapper->chr_rom, 0x2000, chr_rom_size, fp);
	
	return mapper;
}

void destroyMapper000(struct Mapper000* mapper)
{
	free(mapper->chr_rom);
	free(mapper->prg_rom);

	free(mapper);
}

Byte Mapper000_ReadCPU(void* mapper, Word address)
{
	struct Mapper000* sMapper = (struct Mapper000*)mapper;
	Byte val = 0x00;

	if (address >= 0x6000 && address < 0x8000)
	{
		fprintf(stderr, "This Mapper 000 implementation doesnt support FamilyBasic PRG RAM\n");
		exit(-1);
	}
	else if (address >= 0x8000)
	{
		Word effectiveAddress = address - 0x8000;
		effectiveAddress %= 0x4000 * (sMapper->prg_rom_size == 1);

		val = sMapper->prg_rom[effectiveAddress];
	}
	else
	{
		fprintf(stderr, "Cartridge read access violation by the CPU at $%04X\n", address);
		exit(-1);
	}

	return val;
}

Byte Mapper000_ReadPPU(void* mapper, Word address)
{
	struct Mapper000* sMapper = (struct Mapper000*)mapper;
	Byte val = 0x00;

	if (address >= 0x2000)
	{
		fprintf(stderr, "Cartridge read access violation by the PPU at $%04X\n", address);
		exit(-1);
	}
	else 
	{
		val = sMapper->chr_rom[address];
	}

	return val;
}

void Mapper000_WriteCPU(void* mapper, Word address, Byte value)
{
	// nothing
}


void Mapper000_WritePPU(void* mapper, Word address, Byte value)
{
	// nothing
}

void Mapper000_GetPatternTableTexture(void* mapper, SDL_Texture* texture, int index)
{
	struct Mapper000* sMapper = (struct Mapper000*)mapper;

	int pitch;
	void* pixels;
	SDL_LockTexture(texture, NULL, &pixels, &pitch);
	SDL_memcpy(pixels, sMapper->chr_rom + 0x1000 * index, 0x1000);
	SDL_UnlockTexture(texture);
}
