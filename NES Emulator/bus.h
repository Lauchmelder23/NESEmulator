#ifndef _BUS_H_
#define _BUS_H_

#include "types.h"

struct CPU;
struct Cartridge;

// Main communication path for devices and memory in the NES
struct Bus
{
	Byte* ram;
	Byte* io;

	struct CPU* cpu;
	struct Cartridge* cartridge;
};

// Sets up the Bus, allocates memory and creates devices
struct Bus* createBus();

// Destroys the bus, cleans up memory and destroys devices on the Bus
void destroyBus(struct Bus* bus);

// Read/Write to and from the bus
Byte readBus(struct Bus* bus, Word addr);
void writeBus(struct Bus* bus, Word addr, Byte val);


// Ticks the master clock 12 times (i.e. 1 CPU tick & 3 PPU dots)
void tick(struct Bus* bus);

#endif // _BUS_H_