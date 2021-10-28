#ifndef _CARTRIDGE_H_
#define _CARTRIDGE_H_

#include "types.h"
#include "SDL.h"

struct Bus;
struct Mapper;

struct Cartridge
{
	struct
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
	} header;

	struct Mapper* mapper;

	struct Bus* bus;
};

struct Cartridge* createCartridge(struct Bus* parent, const char* filepath);
void destroyCartridge(struct Cartridge* cartridge);

// readCartridge/writeCartridge to and from the cartridge
Byte readCartridgeCPU(struct Cartridge* cartridge, Word addr);
Byte readCartridgePPU(struct Cartridge* cartridge, Word addr);
void writeCartridgeCPU(struct Cartridge* cartridge, Word addr, Byte val);
void writeCartridgePPU(struct Cartridge* cartridge, Word addr, Byte val);

void getPatternTableTexture(struct Cartridge* cartridge, SDL_Texture* texture, int index);

#endif //_CARTRIDGE_H_