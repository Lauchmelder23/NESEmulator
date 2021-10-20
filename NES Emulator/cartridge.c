#include "cartridge.h"
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"

struct INES_Header
{
	Byte nestext[4];
	Byte prg_rom_size;
	Byte chr_rom_size;
	
	struct
	{
		Byte mirror : 1;
		Byte battery : 1;
		Byte trainer : 1;
		Byte ignore_mirror : 1;
		Byte mapper_lower_nibble : 4;
	} Flags6;

	struct
	{
		Byte vs : 1;
		Byte playchoice : 1;
		Byte nes20 : 2;
		Byte mapper_upper_nibble : 4;
	} Flags7;

	Byte prg_ram_size;
	Byte unused[7];
};

struct Cartridge* createCartridge(struct Bus* parent, const char* filepath)
{
	struct Cartridge* cartridge = (struct Cartridge*)malloc(sizeof(struct Cartridge));
	if (cartridge == NULL)
	{
		fprintf(stderr, "Failed to create cartridge.\n");
		exit(1);
	}

	// Open ROM
	FILE* fp = fopen(filepath, "rb");
	
	// Read header
	struct INES_Header header;
	fread(&header, sizeof(header), 1, fp);

	cartridge->prg_rom = (Byte*)malloc(0x4000 * (size_t)header.prg_rom_size);
	if (cartridge->prg_rom == NULL)
	{
		fprintf(stderr, "Failed to allocate PRG memory for cartridge, aborting.\n");
		exit(1);
	}

	cartridge->chr_rom = (Byte*)malloc(0x2000 * (size_t)header.chr_rom_size);
	if (cartridge->prg_rom == NULL)
	{
		fprintf(stderr, "Failed to allocate CHR memory for cartridge, aborting.\n");
		exit(1);
	}

	cartridge->bus = parent;
	return cartridge;
}

void destroyCartridge(struct Cartridge* cartridge)
{
	free(cartridge->chr_rom);
	free(cartridge->prg_rom);

	free(cartridge);
}
