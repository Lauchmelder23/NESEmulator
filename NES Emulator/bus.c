#include "bus.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"

struct Bus* createBus()
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

	// Create CPU and attach it
	bus->cpu = createCPU(bus);

	// Create PPU and attack it
	bus->ppu = createPPU(bus);

	// Create and insert cartridge
	bus->cartridge = createCartridge(bus, "roms/nestest.nes");

	printf("Reset vector: $%x\n", ((Word)readCartridge(bus->cartridge, 0xFFFD) << 8) | (readCartridge(bus->cartridge, 0xFFFC)));

	return bus;
}

void destroyBus(struct Bus* bus)
{
	destroyCartridge(bus->cartridge);
	destroyPPU(bus->ppu);
	destroyCPU(bus->cpu);
	
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
	else if (0x4000 <= addr && addr <= 0x4017)	// I/O space
	{
		val = bus->io[addr - 0x4000];
	}
	else if (0x4020 <= addr && addr <= 0xFFFF)	// Cartridge space
	{
		val = readCartridge(bus->cartridge, addr);
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
	else if (0x4000 <= addr && addr <= 0x4017)	// I/O space
	{
		bus->io[addr - 0x4000] = val;
	}
	else if (0x4020 <= addr && addr <= 0xFFFF)	// Cartridge space
	{
		writeCartridge(bus->cartridge, addr, val);
	}
	else
	{
		fprintf(stderr, "Access violation at $%x", addr);
		exit(1);
	}
}


void tick(struct Bus* bus)
{
	for (int i = 0; i < 3; i++)
		tickPPU(bus->ppu);

	tickCPU(bus->cpu);
}