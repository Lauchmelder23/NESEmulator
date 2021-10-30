#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "types.h"

struct Controller
{
	union
	{
		struct
		{
			Byte A : 1;
			Byte B : 1;
			Byte Select : 1;
			Byte Start : 1;
			Byte Up : 1;
			Byte Down : 1;
			Byte Left : 1;
			Byte Right : 1;
		};

		Byte raw;
	} data;

	Byte strobe;
	Byte latch;
};

Byte pollInput(struct Controller* controller);
void fillRegister(struct Controller* controller);

#endif // _CONTROLLER_H_