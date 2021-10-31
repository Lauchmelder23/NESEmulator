#ifndef _MAPPER_001_
#define _MAPPER_001_

#include <stdio.h>
#include "../types.h"
#include "SDL.h"

struct Mapper001
{
	Byte prg_rom_size;
	Byte chr_rom_size;

	Byte* prg_rom;
	Byte* chr_rom;

	///////////////////////////
	///      REGISTERS      ///
	///////////////////////////
	Byte shiftRegister;
	
	union
	{
		struct
		{
			Byte mirroring : 2;
			Byte prg_bank_mode : 2;
			Byte chr_bank_mode : 1;
			Byte padding : 3;
		};

		Byte raw;
	} control;

	Byte* lowerChrBank;
	Byte* upperChrBank;
	Byte* lowerPrgBank;
	Byte* upperPrgBank;
};

struct Mapper001* createMapper001(Byte prg_rom_size, Byte chr_rom_size, FILE* fp);
void destroyMapper001(struct Mapper001* mapper);

Byte Mapper001_ReadCPU(void* mapper, Word address);
Byte Mapper001_ReadPPU(void* mapper, Word address);
void Mapper001_WriteCPU(void* mapper, Word address, Byte value);
void Mapper001_WritePPU(void* mapper, Word address, Byte value);

void Mapper001_GetPatternTableTexture(void* mapper, SDL_Texture* texture, int index);

#endif // _MAPPER_001_
