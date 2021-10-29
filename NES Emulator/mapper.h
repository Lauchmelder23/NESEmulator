#ifndef _MAPPER_H_
#define _MAPPER_H_

#include <stdio.h>
#include "SDL.h"
#include "types.h"

struct PPU;

struct Mapper
{
	Byte id;
	void* mapperStruct;

	Byte(*read_cpu)(void*, Word);
	Byte(*read_ppu)(void*, Word);
	void(*write_cpu)(void*, Word, Byte);
	void(*write_ppu)(void*, Word, Byte);

	void(*get_pattern_table_texture)(void*, SDL_Texture*, int);
};

#define MapperReadCPU(mapper, addr) mapper->read_cpu(mapper->mapperStruct, addr)
#define MapperReadPPU(mapper, addr) mapper->read_ppu(mapper->mapperStruct, addr)
#define MapperWriteCPU(mapper, addr, val) mapper->write_cpu(mapper->mapperStruct, addr, val)
#define MapperWritePPU(mapper, addr, val) mapper->write_ppu(mapper->mapperStruct, addr, val)
#define GetPatternTableTexture(mapper, texture, index) mapper->get_pattern_table_texture(mapper->mapperStruct, texture, index)

struct Mapper* createMapper(Byte id, Byte prg_rom_size, Byte chr_rom_size, FILE* fp);
void destroyMapper(struct Mapper* mapper);

#endif // _MAPPER_H_
