#ifndef _CPU_H_
#define _CPU_H_

#include "types.h"

struct Bus;

enum Operation
{
	XXX = 0,
	ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI,
	BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI,
	CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR,
	INC, INX, INY, JMP, JSR, LDA, LDX, LDY,
	LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL,
	ROR, RTI, RTS, SBC, SEC, SED, SEI, STA,
	STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,

	ANC, ASR, ARR, DCP, ISC, JAM, LAS, LAX, 
	RLA, RRA, SAX, SBX, SLO, SHA, SHS, SHX, 
	SHY, SRE, XAA
};

enum AddrMode
{
	ACC, ABS, ABX, ABY, IMM, IMP, IND, INDX, INDY, REL, ZPG, ZPX, ZPY
};

struct Opcode
{
	enum Operation op;
	enum AddrMode addr;
	Byte cycles;
	Byte length;
	Byte illegal;

	const char str[4];
};

extern const struct Opcode OPCODE_TABLE[256];


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

	union
	{
		struct
		{
			Byte lo;
			Byte hi;
		};

		Word word;
	} pc;

	char remainingCycles;
	Qword totalCycles;

	Byte fetchedVal;
	Word fetchedAddress;
	char fetchedRelAddress;

	Byte irq;
	Byte nmi;

	const struct Opcode* currentOpcode;

	struct Bus* bus;
};

struct CPU* createCPU(struct Bus* parent);
void destroyCPU(struct CPU* cpu);

int tickCPU(struct CPU* cpu);
void tickInstr(struct CPU* cpu);

void fetch(struct CPU* cpu);
void execute(struct CPU* cpu);

void IRQ(struct CPU* cpu);
void NMI(struct CPU* cpu);

#endif // _CPU_H_
