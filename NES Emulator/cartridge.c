#include "cartridge.h"
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"

struct Cartridge* createCartridge(struct Bus* parent, const char* filepath)
{
	struct Cartridge* cartridge = (struct Cartridge*)malloc(sizeof(struct Cartridge));
	if (cartridge == NULL)
	{
		fprintf(stderr, "Failed to create cartridge.\n");
		exit(1);
	}

	cartridge->memory = NULL;
	cartridge->bus = parent;
	return cartridge;
}

void destroyCartridge(struct Cartridge* cartridge)
{
	// free(cartridge->memory);
	free(cartridge);
}
