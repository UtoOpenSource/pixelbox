#include "chunk.hpp"
#include <time.h>
#include "utils/random.h"

static RNG uuid_rng;

namespace pb {

	// special non-standartized uuid generator :З
	EntityID genEntityID() {
		EntityID id;

		union {
			time_t time;
			uint64_t ival;
			uint8_t bytes[8];
		} tt;

		tt.time = ::time(NULL);
		tt.ival <<= 10;
		tt.ival ^= ::clock() & ((1<<10)-1);

		// 8 bit - time XOR clock bits
		for (int i = 0; i < 8; i++)
			id.v[i] = tt.bytes[8];

		// rest - pseudorng
		for (int i = 0; i < 8; i += 2) {
			uint16_t low = uuid_rng();
			id.v[i + 8] = low;
			id.v[i + 9] = low >> 8;
		}
		return id;
	}

};
