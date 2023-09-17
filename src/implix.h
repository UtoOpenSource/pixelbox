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
// this header used ONLY BY IMPLEMENTATION!

#include "pixel.h"
#include "sqlite3.h"
#pragma once

#define CHUNK_USAGE_VALUE 25
#define GC_LIMIT_PER_TICK 150

#define MAPINT(V) ((V) & (MAPLEN-1))

// specialized murmur hash (was in public domain)
// original : github.com/abrandoned/murmur2/blob/master/MurmurHash2.c
static inline uint32_t murmurhash (uint32_t *data) {
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */
  uint32_t h = 3907472 ^ 4;

	/* Mix 4 bytes at a time into the hash */
	uint32_t k = *(uint32_t*)data;
		
	k *= m;
	k ^= k >> r;
	k *= m;

	h *= m;
	h ^= k;

  /* Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.  */

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
} 

static inline uint32_t hash_function(uint32_t value) {
	//return SuperFastHash((char*) &value, sizeof(value)); 
	return murmurhash(&value);
}
	
#define MAPHASH(V) MAPINT(hash_function(V)) // hash & mapfunc

struct chunk* allocChunk(int16_t x, int16_t y); // +
void freeChunk(struct chunk*); // +

void generateChunk(struct chunk*); 
int  loadChunk(struct chunk*); // don't use
void saveChunk(struct chunk*); // directly!

void addSaveQueue(struct chunk*); // FREES CHUNK AT THE END!!!
void addLoadQueue(struct chunk*); // INSERTS CHUNK IN THE TABLE AT THE END!

struct sqlite3_stmt;
struct sqlite3_stmt* create_statement(sqlite3* db, const char* sql);
int statement_iterator(struct sqlite3_stmt* stmt);

// HASH
struct chunk* findChunk(struct chunkmap* m, int16_t x, int16_t y); // NULL if not found
void insertChunk(struct chunkmap* m, struct chunk* c); 
struct chunk* removeChunk(struct chunkmap* m, struct chunk* c); // returns next chunk if avail.

bool updateChunk(struct chunk* c);
