#include "cartridge.h"
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"
#include "mapper.h"

struct Cartridge* createCartridge(struct Bus* parent, const char* filepath)
{
	struct Cartridge* cartridge = (struct Cartridge*)malloc(sizeof(struct Cartridge));
	if (cartridge == NULL)
	{
		fprintf(stderr, "Failed to create cartridge.\n");
		exit(1);
	}

	FILE* fp = fopen(filepath, "rb");
	
	fread(&cartridge->header, sizeof(cartridge->header), 1, fp);

	Byte mapperID = (cartridge->header.Flags7.mapper_upper_nibble << 4) | cartridge->header.Flags6.mapper_lower_nibble;
	cartridge->mapper = createMapper(mapperID, cartridge->header.prg_rom_size, cartridge->header.chr_rom_size, fp);

	cartridge->bus = parent;

	fclose(fp);
	return cartridge;
}

void destroyCartridge(struct Cartridge* cartridge)
{
	destroyMapper(cartridge->mapper);
	free(cartridge);
}

Byte readCartridgeCPU(struct Cartridge* cartridge, Word addr)
{
	return MapperReadCPU(cartridge->mapper, addr);
}

Byte readCartridgePPU(struct Cartridge* cartridge, Word addr)
{
	return MapperReadPPU(cartridge->mapper, addr);
}

void writeCartridgeCPU(struct Cartridge* cartridge, Word addr, Byte val)
{
	MapperWriteCPU(cartridge->mapper, addr, val);
}

void writeCartridgePPU(struct Cartridge* cartridge, Word addr, Byte val)
{
	MapperWritePPU(cartridge->mapper, addr, val);
}

void getPatternTableTexture(struct Cartridge* cartridge, SDL_Texture* texture, int index)
{
	GetPatternTableTexture(cartridge->mapper, texture, index);
}
