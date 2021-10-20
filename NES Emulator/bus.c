#include "bus.h"
#include <stdlib.h>
#include <stdio.h>

#include "cpu.h"
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

	// Create CPU and attach it
	bus->cpu = createCPU(bus);

	// Create and insert cartridge
	bus->cartridge = createCartridge(bus, "roms/nestest.nes");

	return bus;
}

void destroyBus(struct Bus* bus)
{
	destroyCartridge(bus->cartridge);
	destroyCPU(bus->cpu);
	
	free(bus->ram);
	free(bus);
}

Byte Read(struct Bus* bus, Word addr)
{
	Byte val = 0;

	// Return from the appropriate device depending on the address
	if (addr <= 0x1FFF)	// RAM (or one of the mirrored addresses)
	{
		val = bus->ram[addr & 0x7FF];
	}
	else if (0x4020 <= addr && addr <= 0xFFFF)	// Cartridge space
	{
		// rom->read()
	}
	else
	{
		fprintf(stderr, "Access violation at $%x", addr);
		exit(1);
	}

	return val;
}

void Write(struct Bus* bus, Word addr, Byte val)
{
	// Write to the appropriate memory or device
	if (addr <= 0x1FFF)	// RAM (or one of the mirrored addresses)
	{
		bus->ram[addr & 0x7FF] = val;
	}
	else if (0x4020 <= addr && addr <= 0xFFFF)	// Cartridge space
	{
		// rom->write()
	}
	else
	{
		fprintf(stderr, "Access violation at $%x", addr);
		exit(1);
	}
}
