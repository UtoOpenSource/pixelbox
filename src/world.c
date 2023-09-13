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
	memset(&World.Map, 0, sizeof(World.Map));
	return 0;
}

void freeWorld(void) {
	collectAnything();
	flushWorld();
	if (World.database) sqlite3_close_v2(World.database);
}

static const char* init_sql = 
"PRAGMA cache_size = -16000;"
"PRAGMA journal_mode = MEMORY;"
"PRAGMA auto_vacuum = 2;"
"PRAGMA secure_delete = 0;"
"PRAGMA temp_store = MEMORY;"
"PRAGMA page_size = 32768;"
"CREATE TABLE IF NOT EXISTS PROPERTIES (key STRING PRIMARY KEY, value);" 
"CREATE TABLE IF NOT EXISTS WCHUNKS (id INTEGER PRIMARY KEY, value BLOB);"
"PRAGMA optimize;";

#include <string.h>

int  openWorld(const char* path) {
	if (World.database) sqlite3_close_v2(World.database);

	printf("opening %s...\n", path);
	int stat = sqlite3_open_v2(path, &World.database, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
	if (stat != SQLITE_OK) {
		perror("Can't open database!");
		return -1;
	}
	const char* msg = "no error";
	if (sqlite3_exec(World.database, init_sql, NULL, NULL, &msg) != SQLITE_OK) {
		perror("Can't init database!");
		perror(msg);
	};
	int64_t v = 0;
	if (loadProperty("seed", &v)) setWorldSeed(v);
	if (loadProperty("mode", &v)) World.mode = v;
	return 0;
}


void flushWorld (void) {
	saveProperty("seed", World.seed);
	saveProperty("mode", World.mode);
}

void setWorldPixel(int64_t x, int64_t y, uint8_t val, bool mode) {
	int64_t cx = (uint64_t)x/CHUNK_WIDTH;
	int64_t cy = (uint64_t)y/CHUNK_WIDTH;
	struct chunk* ch;
	ch = getWorldChunk(cx, cy);
	int ax = (uint64_t)x%CHUNK_WIDTH;
	int ay = (uint64_t)y%CHUNK_WIDTH;
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

struct chunk* markWorldUpdate(int64_t x, int64_t y) {
	int64_t cx = (uint64_t)x/CHUNK_WIDTH;
	int64_t cy = (uint64_t)y/CHUNK_WIDTH;

	struct chunk* ch;
	ch = getWorldChunk(cx, cy);
	//int ax = (unsigned int)x%CHUNK_WIDTH;
	//int ay = (unsigned int)y%CHUNK_WIDTH;
	ch->needUpdate = 1;	
	return ch;
}

struct chunk* getWorldChunk(int16_t x, int16_t y) {
	struct chunk* c = findChunk(&World.Map, x, y);
	if (!c) {
		c = allocChunk(x, y);
		addLoadQueue(c); // TODO RETURN NULL AND WAIT!!!
	}
	c->usagefactor = CHUNK_USAGE_VALUE;
	return c;
}

