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
struct worldState World;

#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int initWorld(void) {
	World.database = NULL;
	setWorldSeed(time(NULL));
	memset(&World.map, 0, sizeof(World.map));
	memset(&World.load, 0, sizeof(World.load));
	memset(&World.save, 0, sizeof(World.save));
	memset(&World.update, 0, sizeof(World.update));
	World.map.g = true; // IMPORTANT!
	return 0;
}

#include "profiler.h"

void flushChunks();
void flushSaveLoad();

void flushWorld (void) {
	prof_begin(PROF_DISK);
	saveProperty("seed", World.seed);
	saveProperty("mode", World.mode);
	saveProperty("playtime", World.playtime);

	// heheboi
	for (int i = 0; i < MAPLEN; i++) {
		struct chunk* c = World.load.data[i];
		while (c) {
			struct chunk *f = c;
			c = removeChunk(&World.load, c);
			freeChunk(f);
		}
		World.load.data[i] = NULL;
	}

	flushChunks(); // save all chunks in the World.map hashmap
	while (saveloadTick()) {} // and from World.save hashmap
	flushSaveLoad(); // flush to original file
	prof_end();
}

void initSaveLoad(const char* path);
void freeSaveLoad();

void freeWorld(void) {
	collectAnything(); // cleans up World.map to World.save
	flushWorld(); // flushChunks() is not called there, btw

	prof_begin(PROF_DISK);
	freeSaveLoad();
	prof_end();
}


#include <string.h>

int  openWorld(const char* path) {
	initSaveLoad(path);
	int64_t v = 0;
	if (loadProperty("seed", &v)) setWorldSeed(v);
	if (loadProperty("mode", &v)) World.mode = v;
	if (loadProperty("playtime", &v)) World.playtime = v;
	return 0;
}

void setWorldPixel(int64_t x, int64_t y, uint8_t val, bool mode) {
	int64_t cx = (uint64_t)x/CHUNK_WIDTH;
	int64_t cy = (uint64_t)y/CHUNK_WIDTH;
	struct chunk* ch;
	ch = getWorldChunk(cx, cy);
	if (ch == &empty) return;
	int ax = (uint64_t)x%CHUNK_WIDTH;
	int ay = (uint64_t)y%CHUNK_WIDTH;
	ch->is_changed = 1; // yeah...
	getChunkData(ch, mode)[ax + ay * CHUNK_WIDTH] = val;	
}

uint8_t getWorldPixel(int64_t x, int64_t y, bool mode) {
	int64_t cx = (uint64_t)x/CHUNK_WIDTH;
	int64_t cy = (uint64_t)y/CHUNK_WIDTH;
	struct chunk* ch;
	ch = getWorldChunk(cx, cy);
	int ax = (uint64_t)x%CHUNK_WIDTH;
	int ay = (uint64_t)y%CHUNK_WIDTH;
	return getChunkData(ch, mode)[ax + ay * CHUNK_WIDTH];	
}

struct chunk empty = {0};

struct chunk* markWorldUpdate(int64_t x, int64_t y) {
	int64_t cx = (uint64_t)x/CHUNK_WIDTH;
	int64_t cy = (uint64_t)y/CHUNK_WIDTH;

	struct chunk* ch;
	ch = findChunk(&World.update, cx, cy);
	if (ch) return ch; // already in queue

	// add to the queue
	ch = getWorldChunk(cx, cy);
	if (ch == &empty) {
		empty.pos.axis[0] = cx;
		empty.pos.axis[1] = cy;
		return ch; // no
	}
	insertChunk(&World.update, ch);
	return ch;
}

// noinline!
static struct chunk* slow_loading_path(int16_t x, int16_t y) {
	struct chunk* c;

	// if still loading...
	c = findChunk(&World.load, x, y);
	if (c) {
		empty.pos.axis[0] = x;
		empty.pos.axis[1] = y;
		c->usagefactor = CHUNK_USAGE_VALUE; // 'cause GC will work on loaded chunks
		return &empty;
	}

	// if saving...
	c = findChunk(&World.save, x, y);
	if (c) {
		c->usagefactor = CHUNK_USAGE_VALUE;
		removeChunk(&World.save, c); // important!
		insertChunk(&World.map, c); 
		return c;
	}

	// wait new chunk to be loaded...
	c = allocChunk(x, y);
	c->usagefactor = CHUNK_USAGE_VALUE;
	addLoadQueue(c); // RETURN NULL!!!
	empty.pos.axis[0] = x;
	empty.pos.axis[1] = y;
	return &empty;
}

struct chunk* getWorldChunk(int16_t x, int16_t y) {
	struct chunk* c = findChunk(&World.map, x, y);
	if (!c) {
		return slow_loading_path(x, y); // ok
	}
	c->usagefactor = CHUNK_USAGE_VALUE;
	return c;
}

