#include "log.h"

#include <stdio.h>
#include "bus.h"
#include "cpu.h"
#include "ppu.h"

void logBusState(struct Bus* bus)
{
	char buffer[32];

	Word oldPC = bus->cpu->pc.word - bus->cpu->currentOpcode->length;

	printf("%04X  ", oldPC);

	Word instructionBytes[3];
	for (int i = 0; i < bus->cpu->currentOpcode->length; i++)
	{
		instructionBytes[i] = readBus(bus, oldPC + i, 0);
		sprintf(buffer + 3 * i, "%02X ", instructionBytes[i]);
	}

	printf("%-9s%c%s ", buffer, (bus->cpu->currentOpcode->illegal ? '*' : ' '), bus->cpu->currentOpcode->str);

	switch (bus->cpu->currentOpcode->addr)
	{
	case ACC: sprintf(buffer, "A"); break;
	case ABS: sprintf(buffer, "$%04X",					bus->cpu->fetchedAddress); break;
	case ABX: sprintf(buffer, "$%04X, X -> $%04X",		(instructionBytes[2] << 8) | instructionBytes[1], bus->cpu->fetchedAddress); break;
	case ABY: sprintf(buffer, "$%04X, Y -> $%04X",		(instructionBytes[2] << 8) | instructionBytes[1], bus->cpu->fetchedAddress); break;
	case IMM: sprintf(buffer, "#$%02X",					bus->cpu->fetchedVal); break;
	case IMP: sprintf(buffer, " "); break;
	case IND: sprintf(buffer, "($%04X) -> $%04X",		(instructionBytes[2] << 8) | instructionBytes[1], bus->cpu->fetchedAddress); break;
	case INDX: sprintf(buffer, "($%02X, X) -> $%04X",	instructionBytes[1], bus->cpu->fetchedAddress); break;
	case INDY: sprintf(buffer, "($%02X), Y -> $%04X",	instructionBytes[1], bus->cpu->fetchedAddress); break;
	case REL: sprintf(buffer, "$%02X",					(Byte)bus->cpu->fetchedRelAddress); break;
	case ZPG: sprintf(buffer, "$%02X",					bus->cpu->fetchedAddress); break;
	case ZPX: sprintf(buffer, "$%02X, X -> $%02X",		instructionBytes[1], bus->cpu->fetchedAddress); break;
	case ZPY: sprintf(buffer, "$%02X, Y -> $%02X",		instructionBytes[1], bus->cpu->fetchedAddress); break;
	}

	printf("%-28s", buffer);

	printf("A:%02X X:%02X Y:%02X SP:%02X P:%02X PPU:%3d,%3d CYC:%zu\n", 
		bus->cpu->acc, 
		bus->cpu->x, bus->cpu->y, 
		bus->cpu->sp, 
		bus->cpu->status.raw, 
		bus->ppu->y, bus->ppu->x,
		bus->cpu->totalCycles);
}
