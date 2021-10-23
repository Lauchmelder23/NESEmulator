#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "bus.h"

inline void Push(struct Bus* bus, Byte val)
{
	writeBus(bus, 0x0100 + (bus->cpu->sp--), val);
}

inline Byte Pop(struct Bus* bus)
{
	return readBus(bus, 0x0100 + (++bus->cpu->sp));
}

struct CPU* createCPU(struct Bus* parent)
{
	struct CPU* cpu = (struct CPU*)malloc(sizeof(struct CPU));
	if (cpu == NULL)
	{
		fprintf(stderr, "Failed to create CPU, aborting.\n");
		exit(1);
	}

	// TODO: THIS IS JUST FOR THE TEST ROM
	// cpu->pc.word = 0xC000;
	cpu->pc.word = ((Word)readBus(parent, 0xFFFD) << 8) | readBus(parent, 0xFFFC);

	cpu->status.raw = 0x34;
	cpu->acc = 0;
	cpu->x = 0;
	cpu->y = 0;
	cpu->sp = 0xFD;

	cpu->remainingCycles = 7;
	cpu->totalCycles = 0;

	cpu->bus = parent;
	return cpu;
}

void destroyCPU(struct CPU* cpu)
{
	free(cpu);
}

int tickCPU(struct CPU* cpu)
{
	if (cpu->remainingCycles == -1)	// Jammed
		return 1;

	cpu->remainingCycles--;
	cpu->totalCycles += 1;

	if (cpu->remainingCycles == 0)
	{
		fetch(cpu);
		execute(cpu);
		return 1;
	}

	return 0;
}

void tickInstr(struct CPU* cpu)
{
	while (!tickCPU(cpu));
}

void fetch(struct CPU* cpu)
{
	Byte opcodeVal = readBus(cpu->bus, cpu->pc.word);
	cpu->currentOpcode = OPCODE_TABLE + opcodeVal;

	if (cpu->currentOpcode->op == XXX)
	{
		fprintf(stderr, "Unknown opcode: %02X", opcodeVal);
		exit(1);
	}

	cpu->pc.word++;
	cpu->remainingCycles = cpu->currentOpcode->cycles;

	switch (cpu->currentOpcode->addr)
	{
	case ACC:
		cpu->fetchedVal = cpu->acc;
		return;

	case ABS:
	{
		Byte lo = readBus(cpu->bus, cpu->pc.word++);
		Byte hi = readBus(cpu->bus, cpu->pc.word++);
		cpu->fetchedAddress = ((Word)hi << 8) | lo;
	} break;

	case ABX:
	{
		Byte lo = readBus(cpu->bus, cpu->pc.word++);
		Byte hi = readBus(cpu->bus, cpu->pc.word++);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = addr + cpu->x;
		
		if ((cpu->fetchedAddress & 0xFF00) != (addr & 0xFF00))
			cpu->remainingCycles++;

	} break;

	case ABY:
	{
		Byte lo = readBus(cpu->bus, cpu->pc.word++);
		Byte hi = readBus(cpu->bus, cpu->pc.word++);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = addr + cpu->y;

		if ((cpu->fetchedAddress & 0xFF00) != (addr & 0xFF00))
			cpu->remainingCycles++;

	} break;
	
	case IMM:
		cpu->fetchedVal = readBus(cpu->bus, cpu->pc.word++);
		return;

	case IMP:
		return;

	case IND:
	{
		Byte lo = readBus(cpu->bus, cpu->pc.word++);
		Byte hi = readBus(cpu->bus, cpu->pc.word++);
		Word addr = ((Word)hi << 8);

		cpu->fetchedAddress = ((Word)readBus(cpu->bus, addr | ((lo + 1) & 0xFF)) << 8) | readBus(cpu->bus, addr | lo);

	} break;

	case INDX:
	{
		Byte op = readBus(cpu->bus, cpu->pc.word++);
		Byte lo = readBus(cpu->bus, (op + cpu->x) & 0xFF);
		Byte hi = readBus(cpu->bus, (op + cpu->x + 1) & 0xFF);

		cpu->fetchedAddress = ((Word)hi << 8) | lo;

	} break;

	case INDY:
	{
		Byte op = readBus(cpu->bus, cpu->pc.word++);
		Byte lo = readBus(cpu->bus, op);
		Byte hi = readBus(cpu->bus, (op + 1) & 0xFF);
		Word addr = ((Word)hi << 8) | lo;

		cpu->fetchedAddress = addr + cpu->y;

		if ((cpu->fetchedAddress & 0xFF00) != (addr & 0xFF00))
			cpu->remainingCycles++;

	} break;

	case REL:
	{
		cpu->fetchedRelAddress = readBus(cpu->bus, cpu->pc.word++);
	} break;

	case ZPG:
		cpu->fetchedAddress = readBus(cpu->bus, cpu->pc.word++);
		break;

	case ZPX:
	{
		cpu->fetchedAddress = (readBus(cpu->bus, cpu->pc.word++) + cpu->x) & 0xFF;
	} break;

	case ZPY:
	{
		cpu->fetchedAddress = (readBus(cpu->bus, cpu->pc.word++) + cpu->y) & 0xFF;
	} break;

	}

	cpu->fetchedVal = readBus(cpu->bus, cpu->fetchedAddress);
}

void execute(struct CPU* cpu)
{
	LOG_BUS(cpu->bus);

	switch (cpu->currentOpcode->op)
	{
	case ADC:
	{
		Word result = cpu->acc + cpu->fetchedVal + cpu->status.carry;

		cpu->status.carry = (result > 0xFF);
		cpu->status.overflow = ((~(cpu->acc ^ cpu->fetchedVal) & (cpu->acc ^ result) & 0x80) == 0x80);
		cpu->status.negative = (result >> 7);
		cpu->status.zero = ((result & 0xFF) == 0x00);

		cpu->acc = result & 0xFF;
	} break;

	case AND:
	{
		cpu->acc &= cpu->fetchedVal;

		cpu->status.negative = (cpu->acc >> 7);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	case ASL:
	{
		cpu->status.carry = ((cpu->fetchedVal & 0x80) == 0x80);

		cpu->fetchedVal <<= 1;

		cpu->status.negative = ((cpu->fetchedVal & 0x80) == 0x80);
		cpu->status.zero = (cpu->fetchedVal == 0x00);

		if (!cpu->currentOpcode->addr == ACC)
			writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		else
			cpu->acc = cpu->fetchedVal;
	} break;

	case BCC:
	{
		if (!cpu->status.carry)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BCS:
	{
		if (cpu->status.carry)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BEQ:
	{
		if (cpu->status.zero)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BMI:
	{
		if (cpu->status.negative)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BPL:
	{
		if (!cpu->status.negative)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BVC:
	{
		if (!cpu->status.overflow)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BVS:
	{
		if (cpu->status.overflow)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case BIT:
	{
		cpu->status.negative = (cpu->fetchedVal >> 7);
		cpu->status.overflow = (cpu->fetchedVal >> 6);
		cpu->status.zero = ((cpu->acc & cpu->fetchedVal) == 0x00);
	} break;

	case BNE:
	{
		if (!cpu->status.zero)
		{
			Word target = cpu->pc.word + cpu->fetchedRelAddress;

			cpu->remainingCycles++;
			if ((target & 0xFF00) != (cpu->pc.word & 0xFF00))
				cpu->remainingCycles++;

			cpu->pc.word = target;
		}
	} break;

	case CLC:
	{
		cpu->status.carry = 0;
	} break;

	case CLD:
	{
		cpu->status.decimal = 0;
	} break;

	case CLV:
	{
		cpu->status.overflow = 0;
	} break;

	case CMP:
	{
		Byte result = cpu->acc - cpu->fetchedVal;

		cpu->status.negative = (result >> 7);
		cpu->status.zero = (result == 0x00);
		cpu->status.carry = (cpu->fetchedVal <= cpu->acc);
	} break;

	case CPX:
	{
		Byte result = cpu->x - cpu->fetchedVal;

		cpu->status.negative = (result >> 7);
		cpu->status.zero = (result == 0x00);
		cpu->status.carry = (cpu->fetchedVal <= cpu->x);
	} break;

	case CPY:
	{
		Byte result = cpu->y - cpu->fetchedVal;

		cpu->status.negative = (result >> 7);
		cpu->status.zero = (result == 0x00);
		cpu->status.carry = (cpu->fetchedVal <= cpu->y);
	} break;

	case DCP:
	{
		cpu->fetchedVal--;
		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		Byte result = cpu->acc - cpu->fetchedVal;

		cpu->status.negative = (result >> 7);
		cpu->status.zero = (result == 0x00);
		cpu->status.carry = (cpu->fetchedVal <= cpu->acc);

		cpu->remainingCycles = cpu->currentOpcode->cycles;
	} break;

	case DEC:
	{
		cpu->fetchedVal--;

		cpu->status.negative = ((cpu->fetchedVal & 0x80) == 0x80);
		cpu->status.zero = (cpu->fetchedVal == 0x00);

		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
	} break;

	case DEX:
	{
		cpu->x--;

		cpu->status.negative = ((cpu->x & 0x80) == 0x80);
		cpu->status.zero = (cpu->x == 0x00);
	} break;

	case DEY:
	{
		cpu->y--;

		cpu->status.negative = ((cpu->y & 0x80) == 0x80);
		cpu->status.zero = (cpu->y == 0x80);
	} break;

	case EOR:
	{
		cpu->acc ^= cpu->fetchedVal;

		cpu->status.negative = (cpu->acc >> 7);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	case INC:
	{
		cpu->fetchedVal++;

		cpu->status.negative = ((cpu->fetchedVal & 0x80) == 0x80);
		cpu->status.zero = (cpu->fetchedVal == 0x00);

		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
	} break;

	case INX:
	{
		cpu->x++;

		cpu->status.negative = ((cpu->x & 0x80) == 0x80);
		cpu->status.zero = (cpu->x == 0x00);
	} break;

	case INY:
	{
		cpu->y++;

		cpu->status.negative = ((cpu->y & 0x80) == 0x80);
		cpu->status.zero = (cpu->y == 0x00);
	} break;

	case ISC:
	{
		cpu->fetchedVal++;
		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		Word result = cpu->acc + ~cpu->fetchedVal + cpu->status.carry;

		cpu->status.carry = ((result & 0x8000) != 0x8000);
		cpu->status.overflow = ((~(cpu->acc ^ ~cpu->fetchedVal) & (cpu->acc ^ result) & 0x80) == 0x80);
		cpu->status.negative = ((result & 0x80) == 0x80);
		cpu->status.zero = ((result & 0xFF) == 0x00);

		cpu->acc = result & 0xFF;

		cpu->remainingCycles = cpu->currentOpcode->cycles;
	} break;

	case JMP:
	{
		cpu->pc.word = cpu->fetchedAddress;
	} break;

	case JSR:
	{
		cpu->pc.word--;
		Push(cpu->bus, cpu->pc.hi);
		Push(cpu->bus, cpu->pc.lo);

		cpu->pc.word = cpu->fetchedAddress;
	} break;

	case LAX:
	{
		cpu->acc = cpu->fetchedVal;
		cpu->x = cpu->fetchedVal;

		cpu->status.negative = ((cpu->acc & 0x80) == 0x80);
		cpu->status.zero = (cpu->acc  == 0x00);
	} break;

	case LDA:
	{
		cpu->acc = cpu->fetchedVal;

		cpu->status.negative = (cpu->acc >> 7);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	case LDX:
	{
		cpu->x = cpu->fetchedVal;

		cpu->status.negative = (cpu->x >> 7);
		cpu->status.zero = (cpu->x == 0x00);
	} break;

	case LDY:
	{
		cpu->y = cpu->fetchedVal;

		cpu->status.negative = (cpu->y >> 7);
		cpu->status.zero = (cpu->y == 0x00);
	} break;

	case LSR:
	{
		cpu->status.negative = 0;
		cpu->status.carry = ((cpu->fetchedVal & 0x01) == 0x01);

		cpu->fetchedVal >>= 1;

		cpu->status.zero = (cpu->fetchedVal == 0x00);

		if (!cpu->currentOpcode->addr == ACC)
			writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		else
			cpu->acc = cpu->fetchedVal;
	} break;

	case NOP:
	{

	} break;

	case ORA:
	{
		cpu->acc |= cpu->fetchedVal;

		cpu->status.negative = (cpu->acc >> 7);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	case PHA:
	{
		Push(cpu->bus, cpu->acc);
	} break;

	case PHP:
	{
		Push(cpu->bus, cpu->status.raw | 0b00110000);

	} break;

	case PLA:
	{
		cpu->acc = Pop(cpu->bus);

		cpu->status.negative = (cpu->acc >> 7);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	case PLP:
	{
		cpu->status.raw = Pop(cpu->bus) | 0b00110000;
	} break;

	case RLA:
	{
		Byte oldCarry = cpu->status.carry;
		cpu->status.carry = ((cpu->fetchedVal & 0x80) == 0x80);

		cpu->fetchedVal <<= 1;
		cpu->fetchedVal |= oldCarry;

		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		cpu->acc &= cpu->fetchedVal;

		cpu->status.negative = ((cpu->acc & 0x80) == 0x80);
		cpu->status.zero = (cpu->acc == 0x00);

		cpu->remainingCycles = cpu->currentOpcode->cycles;
	} break;

	case ROL:
	{
		Byte oldCarry = cpu->status.carry;
		cpu->status.carry = ((cpu->fetchedVal & 0x80) == 0x80);

		cpu->fetchedVal <<= 1;
		cpu->fetchedVal |= oldCarry;

		cpu->status.negative = ((cpu->fetchedVal & 0x80) == 0x80);
		cpu->status.zero = (cpu->fetchedVal == 0x00);

		if (!cpu->currentOpcode->addr == ACC)
			writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		else
			cpu->acc = cpu->fetchedVal;
	} break;

	case ROR:
	{
		Byte oldCarry = cpu->status.carry;
		cpu->status.negative = oldCarry;
		cpu->status.carry = ((cpu->fetchedVal & 0x01) == 0x01);

		cpu->fetchedVal >>= 1;
		cpu->fetchedVal |= (oldCarry << 7);

		cpu->status.zero = (cpu->fetchedVal == 0x00);

		if (!cpu->currentOpcode->addr == ACC)
			writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		else
			cpu->acc = cpu->fetchedVal;
	} break;

	case RRA:
	{
		Byte oldCarry = cpu->status.carry;
		cpu->status.carry = ((cpu->fetchedVal & 0x01) == 0x01);

		cpu->fetchedVal >>= 1;
		cpu->fetchedVal |= (oldCarry << 7);

		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);

		Word result = cpu->acc + cpu->fetchedVal + cpu->status.carry;

		cpu->status.carry = (result > 0xFF);
		cpu->status.overflow = ((~(cpu->acc ^ cpu->fetchedVal) & (cpu->acc ^ result) & 0x80) == 0x80);
		cpu->status.negative = (result >> 7);
		cpu->status.zero = ((result & 0xFF) == 0x00);

		cpu->acc = result & 0xFF;
		cpu->remainingCycles = cpu->currentOpcode->cycles;
	} break;

	case RTI:
	{
		cpu->status.raw = Pop(cpu->bus) | 0b00110000;

		cpu->pc.lo = Pop(cpu->bus);
		cpu->pc.hi = Pop(cpu->bus);
	} break;

	case RTS:
	{
		cpu->pc.lo = Pop(cpu->bus);
		cpu->pc.hi = Pop(cpu->bus);
		cpu->pc.word++;
	} break;

	case SAX:
	{
		writeBus(cpu->bus, cpu->fetchedAddress, cpu->acc & cpu->x);
	} break;

	case SBC:
	{
		Word result = cpu->acc + ~cpu->fetchedVal + cpu->status.carry;

		cpu->status.carry = ((result & 0x8000) != 0x8000);
		cpu->status.overflow = ((~(cpu->acc ^ ~cpu->fetchedVal) & (cpu->acc ^ result) & 0x80) == 0x80);
		cpu->status.negative = ((result & 0x80) == 0x80);
		cpu->status.zero = ((result & 0xFF) == 0x00);

		cpu->acc = result & 0xFF;
	} break;

	case SEC:
	{
		cpu->status.carry = 1;
	} break;

	case SED:
	{
		cpu->status.decimal = 1;
	} break;

	case SEI:
	{
		cpu->status.id = 1;
	} break;

	case SLO:
	{
		cpu->status.carry = ((cpu->fetchedVal & 0x80) == 0x80);

		cpu->fetchedVal <<= 1;

		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		cpu->acc |= cpu->fetchedVal;

		cpu->status.negative = ((cpu->acc & 0x80) == 0x80);
		cpu->status.zero = (cpu->acc == 0x00);

		cpu->remainingCycles = cpu->currentOpcode->cycles;
	} break;

	case SRE:
	{
		cpu->status.carry = ((cpu->fetchedVal & 0x01) == 0x01);

		cpu->fetchedVal >>= 1;

		writeBus(cpu->bus, cpu->fetchedAddress, cpu->fetchedVal);
		cpu->acc ^= cpu->fetchedVal;

		cpu->status.negative = ((cpu->acc & 0x80) == 0x80);
		cpu->status.zero = (cpu->acc == 0x00);

		cpu->remainingCycles = cpu->currentOpcode->cycles;
	} break;

	case STA:
	{
		writeBus(cpu->bus, cpu->fetchedAddress, cpu->acc);
		cpu->remainingCycles = cpu->currentOpcode->cycles;	// Correct "oops" cycle (store instr. always have it)
	} break;

	case STX:
	{
		writeBus(cpu->bus, cpu->fetchedAddress, cpu->x);
		cpu->remainingCycles = cpu->currentOpcode->cycles;	// Correct "oops" cycle (store instr. always have it)
	} break;

	case STY:
	{
		writeBus(cpu->bus, cpu->fetchedAddress, cpu->y);
		cpu->remainingCycles = cpu->currentOpcode->cycles;	// Correct "oops" cycle (store instr. always have it)
	} break;

	case TAX:
	{
		cpu->x = cpu->acc;

		cpu->status.negative = ((cpu->x & 0x80) == 0x80);
		cpu->status.zero = (cpu->x == 0x00);
	} break;

	case TAY:
	{
		cpu->y = cpu->acc;

		cpu->status.negative = ((cpu->y & 0x80) == 0x80);
		cpu->status.zero = (cpu->y == 0x00);
	} break;

	case TSX:
	{
		cpu->x = cpu->sp;

		cpu->status.negative = ((cpu->x & 0x80) == 0x80);
		cpu->status.zero = (cpu->x == 0x00);
	} break;

	case TXA:
	{
		cpu->acc = cpu->x;

		cpu->status.negative = ((cpu->acc & 0x80) == 0x80);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	case TXS:
	{
		cpu->sp = cpu->x;
	} break;

	case TYA:
	{
		cpu->acc = cpu->y;

		cpu->status.negative = ((cpu->acc & 0x80) == 0x80);
		cpu->status.zero = (cpu->acc == 0x00);
	} break;

	default:
		fprintf(stderr, "Unknown instruction: %s", cpu->currentOpcode->str);
		exit(1);
		break;
	}
}
