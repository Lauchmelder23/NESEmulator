#include "mapper.h"

#include <stdlib.h>
#include <stdio.h>

#include "mappers/mapper000.h"

struct Mapper* createMapper(Byte id, Byte prg_rom_size, Byte chr_rom_size, FILE* fp)
{
	struct Mapper* mapper = (struct Mapper*)malloc(sizeof(struct Mapper));
	if (mapper == NULL)
	{
		fprintf(stderr, "Failed to create Mapper. Aborting\n");
		exit(-1);
	}

	mapper->id = id;

	switch (id)
	{
	case 0:
	{
		struct Mapper000* mp = createMapper000(prg_rom_size, chr_rom_size, fp);
		mapper->mapperStruct = (void*)mp;

		mapper->read_cpu = &Mapper000_ReadCPU;
		mapper->read_ppu = &Mapper000_ReadPPU;
		mapper->write_cpu = &Mapper000_WriteCPU;
		mapper->write_ppu = &Mapper000_WritePPU;
		mapper->get_pattern_table_texture = &Mapper000_GetPatternTableTexture;
	} break;

	default:
		fprintf(stderr, "Mapper with ID %d is not implemented\n", id);
		exit(-1);
	}
}

void destroyMapper(struct Mapper* mapper)
{
	switch (mapper->id)
	{
	case 0: destroyMapper000((struct Mapper000*)mapper->mapperStruct); break;
	}

	free(mapper);
}
