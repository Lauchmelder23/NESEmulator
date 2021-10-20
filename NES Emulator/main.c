#include "bus.h"

int main()
{
	struct Bus* bus = createBus();
	destroyBus(bus);

	return 0;
}
