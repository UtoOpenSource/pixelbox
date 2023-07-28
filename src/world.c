// see pixelbox.h for copyright notice and license.
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
"PRAGMA cache_size = -8000;"
"PRAGMA journal_mode = WAL;"
"PRAGMA auto_vacuum = 2;"
"PRAGMA secure_delete = 0;"
"PRAGMA temp_store = MEMORY;"
"PRAGMA page_size = 32768;"
"CREATE TABLE IF NOT EXISTS PROPERTIES (key STRING PRIMARY KEY, value);" 
"CREATE TABLE IF NOT EXISTS WCHUNKS (id INTEGER PRIMARY KEY, value BLOB);"
"PRAGMA optimize;";

int  openWorld(const char* path) {
	if (World.database) sqlite3_close_v2(World.database);
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
	}
	if (stat < 1) sqlite3_reset(stmt);
	return stat;
}

void flushWorld (void) {
	//
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

