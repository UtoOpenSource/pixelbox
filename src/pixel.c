// see pixelbox.h for copyright notice and license.
#include "implix.h"
#include <stdlib.h>

struct updater {
	struct chunk* c;
	int8_t  x, y; // DO NOT EDIT THIS COORDS IN CALLBACK AT ALL!!!
	uint8_t v;
};

// (very) slow path
static inline uint8_t _getpixel(int64_t x, int64_t y) {
	return getWorldPixel(x, y, MODE_READ);
}

#include <assert.h>

// fast
static inline uint8_t getpixel(const struct updater* u, int8_t x, int8_t y) {
	uint8_t test =  (uint8_t)x / CHUNK_WIDTH;
	uint8_t test2 = (uint8_t)y / CHUNK_WIDTH;
	if (test || test2) { // if owerflow/underflow (slow path)
		return _getpixel((uint64_t)u->c->pos.axis[0]*CHUNK_WIDTH + x, 
				(uint64_t)u->c->pos.axis[1]*CHUNK_WIDTH + y);
	}
	return getChunkData(u->c, MODE_READ)[x + y * CHUNK_WIDTH]; // fast path
}

static inline void markUpdate(const struct updater* u, int8_t x, int8_t y) {
	uint8_t test =  (uint8_t)x / CHUNK_WIDTH;
	uint8_t test2 = (uint8_t)y / CHUNK_WIDTH;
	if (test || test2) { // if owerflow/underflow (slow path)
		struct chunk* ch 
		= markWorldUpdate((uint64_t)u->c->pos.axis[0]*CHUNK_WIDTH + x, 
			(uint64_t)u->c->pos.axis[1]*CHUNK_WIDTH + y);
		//printf("should be not self update!\n");
		assert(ch != u->c);
	} else {
		assert(y >= 0 && y < CHUNK_WIDTH);
		// do nothing
	}
}

void _initAtoms() {
	
}

static void processor(struct updater* u) {
	uint8_t atom = 0;
	if ((u->v>>2) == 0) {
		atom = getpixel(u, u->x, u->y - 1); // get above us
		if ((atom>>2) % 2 == 1) {
			u->v = atom; // swap
			markUpdate(u, u->x, u->y - 1); // mark chunk-sender (jic)
		}
	} else if ((u->v>>2) % 2 == 1) { 
		atom = getpixel(u, u->x, u->y + 1); // get under us
		if ((atom>>2) == 0) {
			u->v = atom; // swap
			markUpdate(u, u->x, u->y + 1); // mark chunk-reciever
		}
	}
}

#include <string.h>

void updateChunk(struct chunk* c) {
	struct updater u = {c, 0, 0, 0}; // SHOULD be optimized out...
	uint8_t* read = getChunkData(u.c, MODE_READ);
	uint8_t* writ = getChunkData(u.c, MODE_WRITE);
	c->needUpdate = 0; // OK

	for (u.y = 0; u.y < CHUNK_WIDTH; u.y++) {
		for (u.x = 0; u.x < CHUNK_WIDTH; u.x++) {
			u.v = read[u.x + u.y * CHUNK_WIDTH];
			processor(&u);
			if (u.v != read[u.x + u.y * CHUNK_WIDTH])
				c->needUpdate = 1;
			writ[u.x + u.y * CHUNK_WIDTH] = u.v;
		}
	}
}

#include <string.h>
void updateWorld(void) {
	int cnt = 0;

	while (true) {
		for (int i = 0; i < MAPLEN; i++) {
			struct chunk* c = World.Map.data[i];
			while (c) {

				if (c->wasUpdated) { // skip
					c = c->next;
					continue;
				}

				if (c->needUpdate) {
					//printf("chunk %p updated!\n", c);
					updateChunk(c);
					c->wasUpdated = 1;
					cnt++;
				} else {
					memcpy(getChunkData(c, MODE_WRITE), getChunkData(c, MODE_READ), CHUNK_WIDTH*CHUNK_WIDTH);
				}
				c = c->next;	
			}
		} 
		if (cnt == 0) break; // all chunks are done!
		cnt = 0; // try again...
	}

	// remove "was updated" flags
	for (int i = 0; i < MAPLEN; i++) {
		struct chunk* c = World.Map.data[i];
		while (c) {
			c->wasUpdated = 0;
			c = c->next;
		}
	}

	World.wIndex = !World.wIndex;
}

