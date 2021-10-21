#include "bus.h"

int main()
{
	struct Bus* bus = createBus();

	for (;;)
	{
		tick(bus);
	}

	destroyBus(bus);

	return 0;
}
