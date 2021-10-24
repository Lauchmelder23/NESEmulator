#include "bus.h"
#include "ppu.h"

#include <stdio.h>
#include <SDL.h>

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
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

		SDL_Texture* tableTexture = getPatternTableTexture(bus->ppu, 0);
		SDL_Rect target = { 10, 10, 256, 256 };
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getPatternTableTexture(bus->ppu, 1);
		target.x = 256 + 10 + 10;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getNameTableTexture(bus->ppu, 0);
		target.x = 10;
		target.y = 256 + 10 + 10;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		tableTexture = getNameTableTexture(bus->ppu, 1);
		target.x = 256 + 10 + 10;
		SDL_RenderCopy(renderer, tableTexture, NULL, &target);

		SDL_RenderPresent(renderer);
	}

	destroyBus(bus);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
