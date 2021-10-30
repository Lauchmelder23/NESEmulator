#include "bus.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"

struct Bus* createBus(SDL_Renderer* renderer)
{
	struct Bus* bus = (struct Bus*)malloc(sizeof(struct Bus));
	if (bus == NULL)
	{
		fprintf(stderr, "Failed to create Bus object, aborting.");
		exit(1);
	}

	// 2KB of memory, equates to 2048 Bytes (0x800)
	bus->ram = (Byte*)malloc(0x800);
	if (bus->ram == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for NES RAM, aborting.");
		exit(1);
	}
	memset(bus->ram, 0x00, 0x800);

	bus->io = (Byte*)malloc(0x18);
	if (bus->io == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for NES I/O, aborting.");
		exit(1);
	}
	memset(bus->ram, 0x00, 0x18);

	// Create and insert cartridge
	bus->cartridge = createCartridge(bus, "roms/donkeykong.nes");

	// Create CPU and attach it
	bus->cpu = createCPU(bus);

	// Create PPU and attack it
	bus->screen = renderer;
	bus->ppu = createPPU(bus);

	bus->masterClockTimer = 0;

	return bus;
}

void destroyBus(struct Bus* bus)
{
	destroyPPU(bus->ppu);
	destroyCPU(bus->cpu);
	destroyCartridge(bus->cartridge);
	
	free(bus->io);
	free(bus->ram);
	free(bus);
}

Byte readBus(struct Bus* bus, Word addr)
{
	Byte val = 0;

	// Return from the appropriate device depending on the address
	if (addr <= 0x1FFF)	// RAM (or one of the mirrored addresses)
	{
		val = bus->ram[addr & 0x7FF];
	}
	else if (0x2000 <= addr && addr < 0x4000)
	{
		val = ppuRead(bus->ppu, addr & 0x2007);
	}
	else if (0x4000 <= addr && addr <= 0x4017)	// I/O space
	{
		switch (addr)
		{
		case 0x4016:
			val = pollInput(&bus->controller);
			break;
		}
	}
	else if (0x4020 <= addr && addr <= 0xFFFF)	// Cartridge space
	{
		val = readCartridgeCPU(bus->cartridge, addr);
	}
	else
	{
		fprintf(stderr, "Access violation at $%x", addr);
		exit(1);
	}

	return val;
}

void writeBus(struct Bus* bus, Word addr, Byte val)
{
	// writeCartridge to the appropriate memory or device
	if (addr <= 0x1FFF)	// RAM (or one of the mirrored addresses)
	{
		bus->ram[addr & 0x7FF] = val;
	}
	else if (0x2000 <= addr && addr < 0x4000)
	{
		ppuWrite(bus->ppu, addr & 0x2007, val);
	}
	else if (0x4000 <= addr && addr <= 0x4017)	// I/O space
	{
		switch (addr)
		{
		case 0x4016:
			bus->controller.strobe = (val & 0x1);
			if (val & 0x1)
				fillRegister(&bus->controller);
			break;
		}
	}
	else if (0x4020 <= addr && addr <= 0xFFFF)	// Cartridge space
	{
		writeCartridgeCPU(bus->cartridge, addr, val);
	}
	else
	{
		fprintf(stderr, "Access violation at $%x", addr);
		exit(1);
	}
}


int tick(struct Bus* bus)
{
	bus->masterClockTimer++;

	tickPPU(bus->ppu);
	if (bus->masterClockTimer == 3)
	{
		tickCPU(bus->cpu);
		bus->masterClockTimer = 0;
	}
}

int doInstruction(struct Bus* bus)
{
	while(bus->cpu->remainingCycles > 0)
		tick(bus);
	tick(bus);
}

int doFrame(struct Bus* bus)
{
	do tick(bus); while (bus->ppu->x != 0 || bus->ppu->y != 0);
}