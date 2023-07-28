// see pixelbox.h for copyright notice and license.
// this header is used by implementation ONLY!

#include "pixel.h"
#pragma once

#define CHUNK_USAGE_VALUE 25

#define MAPINT(V) ((V) & (MAPLEN-1))
#define MAPHASH(V) MAPINT(V ^ (V*234975434)) // hash & mapfunc

struct chunk* allocChunk(int16_t x, int16_t y); // +
void freeChunk(struct chunk*); // +


void generateChunk(struct chunk*); 
int  loadChunk(struct chunk*);
void saveChunk(struct chunk*);

void addSaveQueue(struct chunk*); // FREES CHUNK AT THE END!!!
void addLoadQueue(struct chunk*); // INSERTS CHUNK IN THE TABLE AT THE END!

struct sqlite3_stmt;
struct sqlite3_stmt* create_statement(const char* sql);
int statement_iterator(struct sqlite3_stmt* stmt);

// HASH
struct chunk* findChunk(struct chunkmap* m, int16_t x, int16_t y); // NULL if not found
void insertChunk(struct chunkmap* m, struct chunk* c); 
struct chunk* removeChunk(struct chunkmap* m, struct chunk* c); // returns next chunk if avail.
