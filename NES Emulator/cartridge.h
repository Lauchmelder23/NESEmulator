#ifndef _CARTRIDGE_H_
#define _CARTRIDGE_H_

#include "types.h"

struct Bus;

struct Cartridge
{
	Byte* prg_rom;
	Byte* chr_rom;

	struct Bus* bus;
};

struct Cartridge* createCartridge(struct Bus* parent, const char* filepath);
void destroyCartridge(struct Cartridge* cartridge);

// readCartridge/writeCartridge to and from the cartridge
Byte readCartridge(struct Cartridge* cartridge, Word addr);
void writeCartridge(struct Cartridge* cartridge, Word addr, Byte val);

#endif //_CARTRIDGE_H_