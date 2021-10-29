#include "bus.h"
#include "ppu.h"
#include "cartridge.h"

#include <stdio.h>
#include <SDL.h>

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1400, 900, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to create SDL_Window.\n");
		exit(1);
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		fprintf(stderr, "Failed to create SDL_Renderer\n");
		exit(1);
	}

	struct Bus* bus = createBus(renderer);

	SDL_Texture* patternTables[2];
	for (int i = 0; i < 2; i++)
	{
		patternTables[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 64, 64);
		if (patternTables[i] == NULL)
		{
			fprintf(stderr, "Failed to create pattern table texture\n");
			exit(-1);
		}
	}

	SDL_Event event;
	int running = 1;
	while(running)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					running = 0;
					break;
				}
			} break;
			}
		}

		doFrame(bus);

		SDL_SetRenderDrawColor(renderer, 20, 0, 20, 0);
		SDL_RenderClear(renderer);

		getPatternTableTexture(bus->cartridge, patternTables[0], 0);
		SDL_Rect target = { 10, 10, 256, 256 };
		SDL_RenderCopy(renderer, patternTables[0], NULL, &target);

		getPatternTableTexture(bus->cartridge, patternTables[1], 1);
		target.x = 256 + 10 + 10;
		SDL_RenderCopy(renderer, patternTables[1], NULL, &target);

		SDL_Texture* tableTexture = getNameTableTexture(bus->ppu, 0);
		target.x = 10;
		target.y = 256 + 10 + 10;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getNameTableTexture(bus->ppu, 1);
		target.x = 256 + 10 + 10;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getRenderedNameTableTexture(bus->ppu, 0);
		target.x = 10;
		target.y = 256 + 10 + 10 + 10 + 256;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getRenderedNameTableTexture(bus->ppu, 1);
		target.x = 256 + 10 + 10;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getScreenTexture(bus->ppu);
		target.x = 10 + 256 + 10 + 256 + 10;
		target.y = 10;
		target.w = 256 * 3;
		target.h = 240 * 3;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		SDL_RenderPresent(renderer);
	}

	for (int i = 0; i < 2; i++)
		SDL_DestroyTexture(patternTables[i]);

	destroyBus(bus);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
