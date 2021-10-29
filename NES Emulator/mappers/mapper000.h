#ifndef _MAPPER_000_
#define _MAPPER_000_

#include <stdio.h>
#include "../types.h"
#include "SDL.h"

struct Mapper000
{
	Byte prg_rom_size;
	Byte chr_rom_size;

	Byte* prg_rom;
	Byte* chr_rom;
};

struct Mapper000* createMapper000(Byte prg_rom_size, Byte chr_rom_size, FILE* fp);
void destroyMapper000(struct Mapper000* mapper);

Byte Mapper000_ReadCPU(void* mapper, Word address);
Byte Mapper000_ReadPPU(void* mapper, Word address);
void Mapper000_WriteCPU(void* mapper, Word address, Byte value);
void Mapper000_WritePPU(void* mapper, Word address, Byte value);

void Mapper000_GetPatternTableTexture(void* mapper, SDL_Texture* texture, int index);

#endif // _MAPPER_000_
