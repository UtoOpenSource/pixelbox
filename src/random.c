// see pixelbox.h for copyright notice and license.
#include "pixel.h"

static uint64_t nextState() {
	int64_t old = World.rngstate;
	World.rngstate = old * 6364136223846793005ULL + (105 | 1);
	return old;
}

int32_t randomNumber(void) {
	uint64_t old = nextState();
	uint32_t sft = (((old >> 18u) ^ old) >> 27u);
	uint32_t rot = (old >> 59u);
	return (sft >> rot) | (sft << ((-(int32_t)rot) & 31));
}

void setSeed(int64_t seed) {
	World.rngstate = 0;
	nextState();
	World.rngstate += seed;
	nextState();
}
