// see pixelbox.h for copyright notice and license.
#include "implix.h"
#include <time.h>
#include <stdlib.h>

struct chunk* allocChunk(int16_t x, int16_t y) {
	struct chunk* c = calloc(sizeof(struct chunk), 1);	
	if (!c) {
		perror("NOMEM! Can't even save world!");
		abort();
	}
	c->pos.axis[0] = x;
	c->pos.axis[1] = y;
	return c;
}

void freeChunk(struct chunk* c) {
	if (c) free(c);
}

uint8_t* getChunkData(struct chunk* c, const bool mode) {
	return c->atoms + (mode == World.wIndex ? 32*32 : 0);
}

#include <string.h>
#include <math.h>

void generateChunk(struct chunk* c) {
	uint8_t* data = getChunkData(c, MODE_READ);
	uint8_t* data2 = getChunkData(c, MODE_WRITE);
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {

			int ax = (x + (int)c->pos.axis[0]*CHUNK_WIDTH);
			int ay = (y + (int)c->pos.axis[1]*CHUNK_WIDTH);
			float v;

			v = noise2(ax/512.0, ay/512.0);
			v -= noise2(ax/1024.0, ay/1024.0);
			if ((v < 0.05 || v > 0.9)) {
				v = noise2(ax/124.0, ay/124.0);
			} else v = 0; 
			data[x + y * CHUNK_WIDTH] = v*255;
			//data2[x + y * CHUNK_WIDTH] = randomNumber();
		}
	}
}

void addSaveQueue(struct chunk* c) {
	saveChunk(c);
	//removeChunk(&World.Map, c); // NOT OK!
	freeChunk(c);
}

void addLoadQueue(struct chunk* c) {
	if (loadChunk(c) <= 0) {
		generateChunk(c);
	}
	c->usagefactor = CHUNK_USAGE_VALUE;
	insertChunk(&World.Map, c); // OK
}

// SQLITE PART 2
#include <sqlite3.h>

int  loadChunk(struct chunk* c) {
	if (World.database) {
		bool loaded = false;
		sqlite3_stmt* stmt = create_statement(
				"SELECT value FROM WCHUNKS WHERE id = ?1;");
		if (!stmt) return -2;
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		while (statement_iterator(stmt) > 0) {
			const uint8_t* data = (uint8_t*)sqlite3_column_blob(stmt, 0);
			if (data) {
				memcpy(getChunkData(c, false), data, 32*32);
				loaded = true;
			}
		}
		sqlite3_finalize(stmt);
		return loaded;
	}
	return -1;
}

void saveChunk(struct chunk* c) {
	if (World.database) {
sqlite3_stmt* stmt = create_statement(
				"INSERT OR REPLACE INTO WCHUNKS VALUES(?1, ?2);");
		if (!stmt) return;
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		sqlite3_bind_blob(stmt, 2, getChunkData(c, MODE_READ), 32*32, SQLITE_STATIC);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
	}
}

