#include "controller.h"
#include <SDL.h>

Byte pollInput(struct Controller* controller)
{
	if (controller->strobe)
	{
		fillRegister(controller);
	}

	Byte ret = 0x1 - (controller->latch & 0x1);	// Controller port is active low
	controller->latch >>= 1;
	return ret;
}

void fillRegister(struct Controller* controller)
{
	const Byte* keyboard = SDL_GetKeyboardState(NULL);

	controller->data.A = keyboard[SDL_SCANCODE_A];
	controller->data.B = keyboard[SDL_SCANCODE_S];
	controller->data.Up = keyboard[SDL_SCANCODE_UP];
	controller->data.Down = keyboard[SDL_SCANCODE_DOWN];
	controller->data.Left = keyboard[SDL_SCANCODE_LEFT];
	controller->data.Right = keyboard[SDL_SCANCODE_RIGHT];
	controller->data.Select = keyboard[SDL_SCANCODE_LSHIFT];
	controller->data.Start = keyboard[SDL_SCANCODE_RETURN];

	controller->latch = controller->data.raw;
}
