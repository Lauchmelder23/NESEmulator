#include "mapper001.h"

struct Mapper001* createMapper001(Byte prg_rom_size, Byte chr_rom_size, FILE* fp)
{
	struct Mapper001* mapper = (struct Mapper001*)malloc(sizeof(struct Mapper001));
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

	mapper->lowerChrBank = mapper->chr_rom;
	mapper->upperChrBank = mapper->chr_rom + 0x1000;
	mapper->lowerPrgBank = mapper->prg_rom;
	mapper->upperPrgBank = mapper->prg_rom + 0x4000;

	return mapper;
}

void destroyMapper001(struct Mapper001* mapper)
{
	free(mapper->chr_rom);
	free(mapper->prg_rom);

	free(mapper);
}

Byte Mapper001_ReadCPU(void* mapper, Word address)
{
	struct Mapper001* sMapper = (struct Mapper000*)mapper;
	Byte val = 0x00;

	if (address >= 0x6000 && address < 0x8000)
	{
		fprintf(stderr, "This Mapper 000 implementation doesnt support FamilyBasic PRG RAM\n");
		exit(-1);
	}
	else if (address >= 0x8000 && address <= 0xBFFF)
	{
		val = sMapper->lowerPrgBank[address & 0x3FFF];
	}
	else if (address >= 0xC000 && address <= 0xFFFF)
	{
		val = sMapper->upperPrgBank[address & 0x3FFF];
	}
	else
	{
		fprintf(stderr, "Cartridge read access violation by the CPU at $%04X\n", address);
		exit(-1);
	}

	return val;
}

Byte Mapper001_ReadPPU(void* mapper, Word address)
{
	struct Mapper001* sMapper = (struct Mapper000*)mapper;
	Byte val = 0x00;

	if (address >= 0x2000)
	{
		fprintf(stderr, "Cartridge read access violation by the PPU at $%04X\n", address);
		exit(-1);
	}
	else if (address < 0x1000 + 0x1000 * (1 - sMapper->control.chr_bank_mode))
	{
		val = sMapper->lowerChrBank[address];
	}
	else
	{
		val = sMapper->upperChrBank[address & 0x0FFF];
	}

	return val;
}

void Mapper001_WriteCPU(void* mapper, Word address, Byte value)
{
	struct Mapper001* sMapper = (struct Mapper000*)mapper;
	if ((value & 0x80) == 0x80)
	{
		sMapper->shiftRegister = 0x10;
		sMapper->control.raw |= 0x0C;
		return;
	}

	if ((sMapper->shiftRegister & 0x1) != 0x1)
	{
		sMapper->shiftRegister >>= 1;
		sMapper->shiftRegister |= ((value & 0x1) << 4);
		return;
	}

	Byte valueToBeWritten = (sMapper->shiftRegister >> 1) | ((value & 0x1) << 4);
	if (address >= 0x8000 && address <= 0x9FFF)
	{
		sMapper->control.raw = valueToBeWritten;
	}
	else if (address >= 0xA000 && address <= 0xBFFF)
	{
		if (sMapper->control.chr_bank_mode == 0x00)	// 8KB mode
			valueToBeWritten &= ~(0x1);	// clear last bit

		sMapper->lowerChrBank = sMapper->chr_rom + (size_t)valueToBeWritten * 0x1000;
	}
	else if (address >= 0xC000 && address <= 0xDFFF)
	{
		if (sMapper->control.chr_bank_mode == 0x00)	// 8KB mode
		{
			return;	// ignore
		}

		sMapper->upperChrBank = sMapper->chr_rom + (size_t)valueToBeWritten * 0x1000;
	}
	else
	{
		switch (sMapper->control.prg_bank_mode)
		{
		case 0: // 32KB mode
		case 1: 
			valueToBeWritten &= ~(0x1);
			sMapper->lowerPrgBank = sMapper->prg_rom + ((size_t)valueToBeWritten & 0xE) * 0x8000;
			sMapper->upperPrgBank = sMapper->lowerPrgBank + 0x4000;
			break;

		case 2: // fix first bank, switch second
			sMapper->lowerPrgBank = sMapper->prg_rom;
			sMapper->upperPrgBank = sMapper->prg_rom + ((size_t)valueToBeWritten & 0xE) * 0x4000;
			break;

		case 3: // fix last bank, switch first
			sMapper->lowerPrgBank = sMapper->prg_rom + ((size_t)valueToBeWritten & 0xE) * 0x4000;
			sMapper->upperPrgBank = sMapper->prg_rom + ((size_t)sMapper->prg_rom_size - 1) * 0x4000;
			break;

		}
	}
}


void Mapper001_WritePPU(void* mapper, Word address, Byte value)
{
	// nothing
}

void Mapper001_GetPatternTableTexture(void* mapper, SDL_Texture* texture, int index)
{
	struct Mapper001* sMapper = (struct Mapper001*)mapper;
	Byte* selectedBank = ((index == 0) ? sMapper->lowerChrBank : sMapper->upperChrBank);

	int pitch;
	void* pixels;
	SDL_LockTexture(texture, NULL, &pixels, &pitch);
	SDL_memcpy(pixels, selectedBank, 0x1000);
	SDL_UnlockTexture(texture);
}
