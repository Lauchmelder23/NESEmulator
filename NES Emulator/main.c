#include "bus.h"

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

	struct Bus* bus = createBus();

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
	}

	destroyBus(bus);

	SDL_Quit();

	return 0;
}
