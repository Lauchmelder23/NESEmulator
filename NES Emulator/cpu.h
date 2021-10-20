#ifndef _CPU_H_
#define _CPU_H_

#include "types.h"

struct Bus;

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

	struct Bus* bus;
};

struct CPU* createCPU(struct Bus* parent);
void destroyCPU(struct CPU* cpu);

#endif // _CPU_H_