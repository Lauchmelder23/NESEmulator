#ifndef _BUS_H_
#define _BUS_H_

#include "types.h"
#include <SDL.h>

struct CPU;
struct PPU;
struct Cartridge;

// Main communication path for devices and memory in the NES
struct Bus
{
	Byte* ram;
	Byte* io;

	struct CPU* cpu;
	struct PPU* ppu;
	struct Cartridge* cartridge;

	Byte masterClockTimer;

	SDL_Renderer* screen;
};

// Sets up the Bus, allocates memory and creates devices
struct Bus* createBus(SDL_Renderer* renderer);

// Destroys the bus, cleans up memory and destroys devices on the Bus
void destroyBus(struct Bus* bus);

// Read/Write to and from the bus
Byte readBus(struct Bus* bus, Word addr);
void writeBus(struct Bus* bus, Word addr, Byte val);


// Ticks the master clock 12 times (i.e. 1 CPU tick & 3 PPU dots)
int tick(struct Bus* bus);
int doInstruction(struct Bus* bus);
int doFrame(struct Bus* bus);

#endif // _BUS_H_