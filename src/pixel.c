/* 
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

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
		assert(ch != u->c);
	} else {
		assert(y >= 0 && y < CHUNK_WIDTH);
		// do nothing
	}
}

void _initAtoms() {
	
}

#define IS_AIR(V) ((V>>2) == 0)
#define IS_SOLID(V) ((V>>2) % 4 == 0)
#define IS_SAND(V)  ((V>>2) % 4 == 1)
#define IS_WATER(V) ((V>>2) % 4 == 2)
#define IS_SPECI(V) ((V>>2) % 4 == 3)

static void processor(struct updater* u, const int stage) {
	uint8_t atom = 0;

	switch (stage) {
		case 0 :
		if (IS_AIR(u->v)) { // air
			atom = getpixel(u, u->x, u->y - 1); // get above us
	
			if (IS_SAND(atom) || IS_WATER(atom)) {
				u->v = atom; // swap
				markUpdate(u, u->x, u->y - 1); // mark chunk-sender
				return;
			}

		} else if (IS_SAND(u->v) || IS_WATER(u->v)) { // sand
			atom = getpixel(u, u->x, u->y + 1); // get under us
	
			if (IS_AIR(atom)) {
				u->v = atom; // swap
				markUpdate(u, u->x, u->y + 1); // mark chunk-reciever
				return;
			}
		}
		break;
		case 1:
		if (IS_AIR(u->v)) { // air
			atom = getpixel(u, u->x-1, u->y);
	
			if (IS_SAND(atom) && IS_SAND(getpixel(u, u->x-1, u->y-1))) {
				u->v = atom; // swap
				markUpdate(u, u->x-1, u->y); // mark chunk-sender
				return;
			}

			int dir = (atom & 1)*2-1;
			if (IS_WATER(atom) && !IS_AIR(getpixel(u, u->x-dir, u->y+1))) {
				u->v = atom; // swap
				markUpdate(u, u->x-dir, u->y); // mark chunk-sender
				return;
			}
	
		} else if (IS_SAND(u->v) && IS_SAND(getpixel(u, u->x, u->y-1))) {
			atom = getpixel(u, u->x+1, u->y); 
	
			if (IS_AIR(atom)) {
				u->v = atom; // swap
				markUpdate(u, u->x+1, u->y); // mark chunk-reciever
				return;
			}
		} else if (IS_WATER(u->v) && !IS_AIR(getpixel(u, u->x, u->y+1))) {
			int dir = (u->v & 1)*2-1;
			atom = getpixel(u, u->x+dir, u->y); 
	
			if (IS_AIR(atom)) {
				u->v = atom; // swap
				markUpdate(u, u->x+dir, u->y); // mark chunk-reciever
				return;
			}
		}
		break;
		case 2:
		if (IS_AIR(u->v)) { // air
			atom = getpixel(u, u->x+1, u->y);
	
			if (IS_SAND(atom) && IS_SAND(getpixel(u, u->x+1, u->y-1))) {
				u->v = atom; // swap
				markUpdate(u, u->x+1, u->y); // mark chunk-sender
				return;
			}

			if (IS_WATER(atom) && !IS_AIR(getpixel(u, u->x+1, u->y+1))) {
				u->v = atom; // swap
				markUpdate(u, u->x+1, u->y); // mark chunk-sender
				return;
			}
	
		} else if (IS_SAND(u->v) && IS_SAND(getpixel(u, u->x, u->y-1))) {
			atom = getpixel(u, u->x-1, u->y); 
	
			if (IS_AIR(atom)) {
				u->v = atom; // swap
				markUpdate(u, u->x-1, u->y); // mark chunk-reciever
				return;
			}
		} else if (IS_WATER(u->v) && !IS_AIR(getpixel(u, u->x, u->y+1))) {
			atom = getpixel(u, u->x-1, u->y); 
	
			if (IS_AIR(atom)) {
				u->v = atom; // swap
				markUpdate(u, u->x-1, u->y); // mark chunk-reciever
				return;
			}
		}
		break;
		case 3:

		break;
		case 4:

		break;
		case 5:

		break;
		case 6:

		break;
		case 7:

		break;
		default: break;
	}
}

#include <string.h>
#include <raylib.h>


bool updateChunk(struct chunk* c, const int stage) {
	struct updater u = {c, 0, 0, 0}; // SHOULD be optimized out...
	uint8_t* read = getChunkData(u.c, MODE_READ);
	uint8_t* writ = getChunkData(u.c, MODE_WRITE);
	bool need  = false;

	for (u.y = 0; u.y < CHUNK_WIDTH; u.y++) {
		for (u.x = 0; u.x < CHUNK_WIDTH; u.x++) {
			u.v = read[u.x + u.y * CHUNK_WIDTH];
			processor(&u, stage);
			if (u.v != read[u.x + u.y * CHUNK_WIDTH])
				need = true;
			writ[u.x + u.y * CHUNK_WIDTH] = u.v;
		}
	}
	return need;
}

#include <string.h>

#define TPS 64
#define MIN_TICK (1.0/(double)TPS)
#define MAX_STAGES 7
static double old_time = 0.0;

void updateWorld(void) {
	double dt = GetTime() - old_time;
	if (dt < MIN_TICK) { // too early
		for (int i = 0; i < MAPLEN; i++) {
			struct chunk* c = World.update.data[i];
			while (c) {
				c->usagefactor = CHUNK_USAGE_VALUE;
				c = c->next2;
			}
		}
		return;
	};
	int repeat = dt/MIN_TICK;
	assert(repeat);
	old_time = GetTime();

	if (!World.is_update_enabled) { // yeah
		// cleanup map
		for (int i = 0; i < MAPLEN; i++) {
			World.update.data[i] = NULL; // hehehe
		}
		return;
	}

	for (; repeat > 0; repeat--) {
		int cnt = 0;
		for (int stage = 0; stage < MAX_STAGES; stage++) {
			int inncnt;
			repeat_stage:
			inncnt = 0;
			for (int i = 0; i < MAPLEN; i++) {
				struct chunk* c = World.update.data[i];
				while (c) {
					if ((c->wasUpdated & (1 << stage)) != 0) { // skip
						c = c->next2;
						continue;
					}
					
					if (updateChunk(c, stage)) { // done
						c->is_changed = 1;
						c->wasUpdated |= (1 << stage);
						cnt++;
						inncnt++;
					};
					c = c->next2;	
				}
			}
			if (inncnt != 0) goto repeat_stage;
			// swap buffers
			for (int i = 0; i < MAPLEN; i++) {
				struct chunk* c = World.update.data[i];
				while (c) {
					if ((c->wasUpdated & (1 << stage))) {
						c->wIndex = !c->wIndex;
					}
					c = c->next2;
				}
			}

		}

		// remove "was updated" flag and not updated chunks
		for (int i = 0; i < MAPLEN; i++) {
			struct chunk* c = World.update.data[i];
			struct chunk* p = NULL;
			while (c) {
	
				if (!c->wasUpdated) {
					if (p) p->next2 = c->next2;
					else World.update.data[i] = c->next2; 
					c = c->next2; // p stays the same
					continue;
				}
				c->wasUpdated = 0;
				p = c; // now we are previous
				c = c->next2;
			}
		}
		if (cnt == 0) break;
	}
}
