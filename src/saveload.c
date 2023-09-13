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
#include <stdio.h>
#include <string.h>

static int load_i = 0;
static int save_i = 0;

#define SCORE_SAVE 2
#define SCORE_GEN  1
#define SCORE_LOAD 1
#define SCORE_MAX  5

bool saveloadTick() {
	bool done_something = false;
	struct chunkmap* m = &World.load;
	int limit = 0;

	while (load_i < MAPLEN) {
		while (m->data[load_i]) {
			if (limit >= SCORE_MAX) goto skip_load; // exit NOW!
			struct chunk* c = m->data[load_i];
			struct chunk* n = c->next2; // get and remove
			m->data[load_i] = n;
			done_something = true;
	
			if (loadChunk(c) <= 0) {
				generateChunk(c);
				limit += SCORE_GEN;
			} else limit += SCORE_LOAD;

			c->usagefactor = CHUNK_USAGE_VALUE;
			insertChunk(&World.map, c); // OK
			//limit++;
		}
		load_i++;
	}
	load_i = 0; // restart

	skip_load:
	m = &World.save;
	limit = 0;

	while (save_i < MAPLEN) {
		while (m->data[save_i]) {
			if (limit >= SCORE_MAX) goto skip_save; // exit NOW!
			struct chunk* c = m->data[save_i];
			struct chunk* n = c->next2; // get and remove
			m->data[save_i] = n;
			done_something = true;

			// don't save unchanged chunks
			if (c->is_changed) saveChunk(c);

			// free in that case!
			if (!findChunk(&World.map, c->pos.axis[0], c->pos.axis[1])) {
				freeChunk(c);
			}

			limit += SCORE_SAVE;
		}
		save_i++;
	}
	save_i = 0; // restart

	skip_save:
	return done_something;
}

void addSaveQueue(struct chunk* c) {
	if (!findChunk(&World.save, c->pos.axis[0], c->pos.axis[1]))
		insertChunk(&World.save, c);
}

void addLoadQueue(struct chunk* c) {
	//if (!findChunk(&World.load, c->pos.axis[0], c->pos.axis[1]))
	c->is_changed = 0;
	insertChunk(&World.load, c);
}

#include "profiler.h"

// SQLITE PART IS HERE NOW! :З
// but initialization is still in the world.c :/
#include <sqlite3.h>

int  loadChunk(struct chunk* c) {
	if (World.database) {
		prof_begin(PROF_DISK);
		bool loaded = false;
		sqlite3_stmt* stmt = create_statement(
				"SELECT value FROM WCHUNKS WHERE id = ?1;");
		if (!stmt) {
			prof_end();
			return -2;
		}
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		while (statement_iterator(stmt) > 0) {
			const uint8_t* data = (uint8_t*)sqlite3_column_blob(stmt, 0);
			if (data) {
				memcpy(getChunkData(c, false), data, CHUNK_WIDTH*CHUNK_WIDTH);
				loaded = true;
			}
		}
		sqlite3_finalize(stmt);
		prof_end();
		return loaded;
	}
	return -1;
}

void saveChunk(struct chunk* c) {
	if (World.database) {
		prof_begin(PROF_DISK);
sqlite3_stmt* stmt = create_statement(
				"INSERT OR REPLACE INTO WCHUNKS VALUES(?1, ?2);");
		if (!stmt) {
			prof_end();
			return;
		}
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		sqlite3_bind_blob(stmt, 2, getChunkData(c, MODE_READ), CHUNK_WIDTH*CHUNK_WIDTH, SQLITE_STATIC);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
		prof_end();
	}
}

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

sqlite3_stmt* create_statement(const char* sql) {
	sqlite3_stmt* ptr;

	prof_begin(PROF_LOAD_INIT);

	int err	= sqlite3_prepare_v3(World.database, sql, -1, 0,
		&ptr, (const char**)0);

	prof_end();

	if (err != SQLITE_OK) {
		perror(sqlite3_errmsg(World.database));
		return NULL;
	}
	return ptr;
}


// returns 1 if there is still data to process
// returns 0 if statement completed successfully
// returns -1 if error occured
int statement_iterator(sqlite3_stmt* stmt) {

	int attemts = 0, res = 0;
	retry :
	res = sqlite3_step(stmt);
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
