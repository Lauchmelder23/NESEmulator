#ifndef _CPU_H_
#define _CPU_H_

#include "types.h"

struct Bus;

typedef enum
{
	XXX = 0,
	ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI,
	BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI,
	CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR,
	INC, INX, INY, JMP, JSR, LDA, LDX, LDY,
	LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL,
	ROR, RTI, RTS, SBC, SEC, SED, SEI, STA,
	STX, STY, TAX, TAY, TSX, TXA, TXS, TYA
} Operation;

typedef enum 
{
	ACC, ABS, ABX, ABY, IMM, IMP, IND, XIN, INY, REL, ZPG, ZPX, ZPY
} AddrMode;

struct Opcode
{
	Operation op;
	AddrMode addr;
	Byte cycles;

	const char str[4];
};

const struct Opcode OPCODE_MATRIX[512];


struct CPU
{
	Byte acc;
	Byte x, y;
	Byte sp;

	union
	{
		struct
		{
			Byte carry : 1;
			Byte zero : 1;
			Byte id : 1;
			Byte decimal : 1;
			Byte unused : 2;
			Byte overflow : 1;
			Byte negative : 1;
		};

		Byte raw;
	} status;

	Word pc;

	Byte remainingCycles;
	Byte fetchedVal;
	Word fetchedAddress;
	const struct Opcode* currentOpcode;

	struct Bus* bus;
};

struct CPU* createCPU(struct Bus* parent);
void destroyCPU(struct CPU* cpu);

void tickCPU(struct CPU* cpu);
void tickInstr(struct CPU* cpu);

void fetch(struct CPU* cpu);
void execute(struct CPU* cpu);

#endif // _CPU_H_