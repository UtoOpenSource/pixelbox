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

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int initWorld(void) {
	World.database = NULL;
	setWorldSeed(time(NULL));
	World.wIndex = false;
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

static bool getprop(const char* name, int64_t *out) {
	if (World.database) {
		bool loaded = false;
		sqlite3_stmt* stmt = create_statement(
				"SELECT value FROM PROPERTIES WHERE key = ?1;");
		if (!stmt) {
			perror(sqlite3_errmsg(World.database));
			return false;
		}
		sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
		while (statement_iterator(stmt) > 0) {
			*out = sqlite3_column_int64(stmt, 0);
			loaded = true;
		}
		sqlite3_finalize(stmt);
		return loaded;
	}
	return false;
}

bool loadProperty(const char* k, int64_t *v) {
	return getprop(k, v);
}

static bool setprop(const char* name, int64_t v) {
	if (World.database) {
		sqlite3_stmt* stmt = create_statement(
				"INSERT OR REPLACE INTO PROPERTIES VALUES(?1, ?2);");
		if (!stmt) {
			perror(sqlite3_errmsg(World.database));
			return false;
		}
		sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
		sqlite3_bind_int64(stmt, 2, v);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
		return true;
	}
	fprintf(stderr, "property saving failture!");
	return false;
}

bool saveProperty(const char* k, int64_t v) {
	return setprop(k, v);
}

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
	if (getprop("seed", &v)) setWorldSeed(v);
	if (getprop("mode", &v)) World.mode = v;
	return 0;
}

sqlite3_stmt* create_statement(const char* sql) {
	sqlite3_stmt* ptr;
	int err	= sqlite3_prepare_v3(World.database, sql, -1, 0,
		&ptr, (const char**)0);
	if (err != SQLITE_OK) {
		perror(sqlite3_errmsg(World.database));
		return NULL;
	}
	return ptr;
}

// returns 1 if there is still data to process
// returns 0 if statement cmpleted successfully
// returns -1 if error occured
int statement_iterator(sqlite3_stmt* stmt) {
	int attemts = 0;
	retry :
	int res = sqlite3_step(stmt);
	if (res == SQLITE_BUSY) {
		attemts++;
		if (attemts < 100) goto retry;
	}
	if (res == SQLITE_DONE) {
		// DONE
	}
	int stat;
	if (res == SQLITE_DONE) stat = 0;
	else if (res == SQLITE_ROW) stat = 1;
	else {
		stat = -1;
		perror(sqlite3_errstr(res));
		perror(sqlite3_errmsg(World.database));
	}
	if (stat < 1) sqlite3_reset(stmt);
	return stat;
}

void flushWorld (void) {
	setprop("seed", World.seed);
	setprop("mode", World.mode);
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

