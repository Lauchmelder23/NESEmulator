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

	if (cpu->currentOpcode->op == XXX)
	{
		fprintf(stderr, "Unknown opcode: %x", opcodeVal);
		exit(1);
	}

	cpu->remainingCycles = cpu->currentOpcode->cycles;

	switch (cpu->currentOpcode->addr)
	{
	case ACC:
		cpu->fetchedVal = cpu->acc;
		break;

	case ABS:
	{
		Byte lo = readBus(cpu->bus, cpu->pc++);
		Byte hi = readBus(cpu->bus, cpu->pc++);
		cpu->fetchedAddress = ((Word)hi << 8) | lo;
	} break;

	case ABX:
	{
		Byte lo = readBus(cpu->bus, cpu->pc++);
		Byte hi = readBus(cpu->bus, cpu->pc++);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = addr + cpu->x;
		
		if ((cpu->fetchedAddress & 0xFF00) != (addr & 0xFF00))
			cpu->remainingCycles++;

	} break;

	case ABY:
	{
		Byte lo = readBus(cpu->bus, cpu->pc++);
		Byte hi = readBus(cpu->bus, cpu->pc++);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = addr + cpu->y;

		if ((cpu->fetchedAddress & 0xFF00) != (addr & 0xFF00))
			cpu->remainingCycles++;

	} break;
	
	case IMM:
		cpu->fetchedVal = readBus(cpu->bus, cpu->pc++);
		break;

	case IMP:
		break;

	case IND:
	{
		Byte lo = readBus(cpu->bus, cpu->pc++);
		Byte hi = readBus(cpu->bus, cpu->pc++);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = readBus(cpu->bus, addr);

	} break;

	case INDX:
	{
		Byte op = readBus(cpu->bus, cpu->pc++);
		Byte lo = readBus(cpu->bus, op + cpu->x);
		Byte hi = readBus(cpu->bus, op + cpu->x + 1);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = readBus(cpu->bus, addr);

	} break;

	case INDY:
	{
		Byte op = readBus(cpu->bus, cpu->pc++);
		Byte lo = readBus(cpu->bus, op);
		Byte hi = readBus(cpu->bus, op + 1);
		Word addr = ((Word)hi << 8) | lo;
		addr = readBus(cpu->bus, addr);

		cpu->fetchedAddress = addr + cpu->y;

		if ((cpu->fetchedAddress & 0xFF00) != (addr & 0xFF00))
			cpu->remainingCycles++;

	} break;

	case REL:
	{
		cpu->fetchedRelAddress = readBus(cpu->bus, cpu->pc++);
	} break;

	case ZPG:
		cpu->fetchedAddress = readBus(cpu->bus, cpu->pc++);
		break;

	case ZPX:
	{
		cpu->fetchedAddress = readBus(cpu->bus, cpu->pc++) + cpu->x;
		cpu->fetchedAddress |= 0xFF;
	} break;

	case ZPY:
	{
		cpu->fetchedAddress = readBus(cpu->bus, cpu->pc++) + cpu->y;
		cpu->fetchedAddress |= 0xFF;
	} break;

	}
}

void execute(struct CPU* cpu)
{
	switch (cpu->currentOpcode->op)
	{
	case JMP:
	{
		cpu->pc = cpu->fetchedAddress;
	} break;

	default:
		fprintf(stderr, "Unknown instruction: %s", cpu->currentOpcode->str);
		exit(1);
		break;
	}
}
