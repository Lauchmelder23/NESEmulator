#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"

struct CPU* createCPU(struct Bus* parent)
{
	struct CPU* cpu = (struct CPU*)malloc(sizeof(struct CPU));
	if (cpu == NULL)
	{
		fprintf(stderr, "Failed to create CPU, aborting.\n");
		exit(1);
	}

	// TODO: THIS IS JUST FOR THE TEST ROM
	cpu->pc = 0xC000;

	cpu->bus = parent;
	return cpu;
}

void destroyCPU(struct CPU* cpu)
{
	free(cpu);
}

void tickCPU(struct CPU* cpu)
{
	if (cpu->fetchedVal != 0)
	{
		cpu->fetchedVal--;
		return;
	}

	fetch(cpu);
	execute(cpu);
}

void tickInstr(struct CPU* cpu)
{
	while (cpu->remainingCycles > 1)
		tickCPU(cpu);
}

void fetch(struct CPU* cpu)
{
	Byte opcodeVal = readBus(cpu->bus, cpu->pc);
	cpu->currentOpcode = OPCODE_TABLE + opcodeVal;

	if (cpu->currentOpcode->op == Unknown)
	{
		fprintf(stderr, "Unknown opcode: %x", opcodeVal);
		exit(1);
	}


}

void execute(struct CPU* cpu)
{
}
