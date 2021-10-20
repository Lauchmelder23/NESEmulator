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

	cpu->bus = parent;
	return cpu;
}

void destroyCPU(struct CPU* cpu)
{
	free(cpu);
}
