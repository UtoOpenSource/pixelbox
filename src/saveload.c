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

#define SCORE_SAVE 4
#define SCORE_GEN  1
#define SCORE_LOAD 2
#define SCORE_MAX  10

#include <assert.h>
// SQLITE PART IS HERE NOW! :З
#include <sqlite3.h>

static const char* init_sql = 
"PRAGMA cache_size = -16000;"
"PRAGMA journal_mode = MEMORY;"
"PRAGMA synchronous = 0;" // TODO : may be 1 ok?
"PRAGMA auto_vacuum = 0;"
"PRAGMA secure_delete = 0;"
"PRAGMA temp_store = MEMORY;"
"PRAGMA page_size = 32768;"
"PRAGMA integrity_check;"
"CREATE TABLE IF NOT EXISTS PROPERTIES (key STRING PRIMARY KEY, value);" 
"CREATE TABLE IF NOT EXISTS WCHUNKS (id INTEGER PRIMARY KEY, value BLOB);"
;

static const char* flush_sql = 
"VACUUM;"
"PRAGMA optimize;";

static void badWorldVersion() {
	perror("Bad world version!");
	fprintf(stderr, "Pixelbox will abort loading of this world!\n");
	fprintf(stderr, "You can try to add PROPS.version manually, if\n");
	fprintf(stderr, "you're sure that's a error, but if world does \n");
	fprintf(stderr, "corrupt - that's your fault!\n");
}

#include "version.h"

bool checkVersion(sqlite3* db, bool init) {
	sqlite3_stmt* stmt;
	if (init) {
		stmt = create_statement(db,
			"INSERT OR IGNORE INTO PROPERTIES VALUES('version', ?1);\n"
		);
		if (!stmt) {
			perror(sqlite3_errmsg(db));
			return false;
		}
		sqlite3_bind_int64(stmt, 1, PBOX_NUMERIC_VERSION);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
	}

	stmt = create_statement(db,
		"SELECT value FROM PROPERTIES WHERE key = 'version';"
	);

	if (!stmt) {
		perror(sqlite3_errmsg(db));
		return false;
	};

	int64_t v = 0;
	while (statement_iterator(stmt) > 0) {
		if (sqlite3_column_count(stmt) > 0)
			v = sqlite3_column_int64(stmt, 0);
	}
	sqlite3_finalize(stmt);

	fprintf(stderr, "versions : world=%li, game=%li\n", v, (int64_t)PBOX_NUMERIC_VERSION);
	return v == PBOX_NUMERIC_VERSION;
}

void initSaveLoad(const char* path) {
	if (World.database) sqlite3_close_v2(World.database);

	int stat = sqlite3_open_v2(
		path, &World.database,
		SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL
	);

	if (stat != SQLITE_OK) {
		perror("Can't open database!");
		return;
	}

	char* msg = "no error";
	if (sqlite3_exec(World.database, init_sql, NULL, NULL, &msg) != SQLITE_OK) {
		perror("Can't init database!");
		fprintf(stderr, "You will get into the null world now.\n");
		perror(msg);
	};

	if (!checkVersion(World.database, true)) {	
		badWorldVersion();
		fprintf(stderr, "You will get into the null world now.\n");
		sqlite3_close_v2(World.database);
		World.database = NULL;
	};
}

void freeSaveLoad() {
	if (World.database) sqlite3_close_v2(World.database);
}

static bool getprop(sqlite3* db, const char* name, int64_t *out);
static bool setprop(sqlite3* db, const char* name, int64_t out);

bool getWorldInfo(const char* path, uint64_t *time, int *mode) {
	sqlite3* db = NULL;
	int stat = sqlite3_open_v2(path, &db, SQLITE_OPEN_READONLY, NULL);

	if (stat != SQLITE_OK) return false;

	bool res = false;

	if (!checkVersion(db, false)) goto fail;

	if (!getprop(db, "playtime", time)) {
		perror("no playtime! :(");
		goto fail;
	}
	if (!getprop(db, "mode", mode)) {
		perror("no mode! :(");
		goto fail;
	}

	res = true;

fail:
	sqlite3_close_v2(db);
	return res;
}

void flushSaveLoad() {
	const char* msg = "no error";
	if (!World.database) return; // exit
	if (sqlite3_exec(World.database, flush_sql, NULL, NULL, &msg) != SQLITE_OK) {
		perror("Can't finalize database properly!");
		perror(msg);
	};
}

int  loadChunk(struct chunk* c) {
	if (World.database) {
		bool loaded = false;
		sqlite3_stmt* stmt = create_statement(World.database,
				"SELECT value FROM WCHUNKS WHERE id = ?1;");
		if (!stmt) {
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
		return loaded;
	}
	return -1;
}

void saveChunk(struct chunk* c) {
	if (World.database) {
sqlite3_stmt* stmt = create_statement(World.database,
				"INSERT OR REPLACE INTO WCHUNKS VALUES(?1, ?2);");
		if (!stmt) {
			return;
		}
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		sqlite3_bind_blob(stmt, 2, getChunkData(c, MODE_READ), CHUNK_WIDTH*CHUNK_WIDTH, SQLITE_STATIC);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
	}
}

static bool getprop(sqlite3* db, const char* name, int64_t *out) {
	if (db) {
		bool loaded = false;
		sqlite3_stmt* stmt = create_statement(db,
				"SELECT value FROM PROPERTIES WHERE key = ?1;");
		if (!stmt) {
			perror(sqlite3_errmsg(db));
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
	return getprop(World.database, k, v);
}

static bool setprop(sqlite3* db, const char* name, int64_t v) {
	if (db) {
		sqlite3_stmt* stmt = create_statement(db, 
				"INSERT OR REPLACE INTO PROPERTIES VALUES(?1, ?2);");
		if (!stmt) {
			perror(sqlite3_errmsg(db));
			return false;
		}
		sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
		sqlite3_bind_int64(stmt, 2, v);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
		return true;
	}
	return false;
}

bool saveProperty(const char* k, int64_t v) {
	return setprop(World.database, k, v);
}

sqlite3_stmt* create_statement(sqlite3* db, const char* sql) {
	sqlite3_stmt* ptr;

	int err	= sqlite3_prepare_v3(db, sql, -1, 0,
		&ptr, (const char**)0);

	if (err != SQLITE_OK) {
		perror(sqlite3_errmsg(db));
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

			assert(c->usagefactor > 0); // should be true

			if (loadChunk(c) <= 0) {
				generateChunk(c);
				limit += SCORE_GEN;
			} else limit += SCORE_LOAD;

			c->usagefactor = CHUNK_USAGE_VALUE;
			insertChunk(&World.map, c); // OK
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
			if (c->is_changed) {
				saveChunk(c);
				limit += SCORE_SAVE;
			}

			// free in that case!
			assert(!findChunk(&World.map, c->pos.axis[0], c->pos.axis[1]));
			{
				freeChunk(c);
			}

		}
		save_i++;
	}
	save_i = 0; // restart

	skip_save:
	return done_something;
}

void addSaveQueue(struct chunk* c) {
	if (findChunk(&World.save, c->pos.axis[0], c->pos.axis[1])) return;
	if (findChunk(&World.update, 	c->pos.axis[0], c->pos.axis[1])) {
		removeChunk(&World.update, c); // important
	}
	insertChunk(&World.save, c);
}

void addLoadQueue(struct chunk* c) {
	//if (!findChunk(&World.load, c->pos.axis[0], c->pos.axis[1]))
	c->is_changed = 0;
	insertChunk(&World.load, c);
}

// we CAN'T just add all chunks there to save queue
// (cause they may be in update queue already)
// so, we will do direct approach there :p
void flushChunks() {
	struct chunkmap* m = &World.map;
	for (int i = 0; i < MAPLEN; i++) {
		struct chunk* c = m->data[i];
		while (c) {
			// don't save unchanged chunks
			if (c->is_changed) saveChunk(c);
			c->is_changed = 0; // 'cause yeah
			c = c->next;
		}
	}
}


