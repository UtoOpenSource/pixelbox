/*
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Pixelbox INTERNAL API
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>
 */

#include "pbapi.h"
#pragma once

#define CHUNK_USAGE_VALUE 25
static_assert(CHUNK_USAGE_VALUE > 1);

// deprecated!
//#define MAPINT(V, MAPLEN) ((V) & (MAPLEN - 1))

/* structures implementation */
struct Chunk {
	struct chunk *next, *next2;	 // next2 for secondary hashmap!
	union packpos pos;
	struct atoms buffers[2];
	bool wIndex;	// writable atoms index
	uint8_t is_changed : 1;
	int8_t usagefactor;	 // GC
};

// #define MAPLEN 128 // must be pow of 2!
#define MINILEN 128	 // must be pow of 2!

// specialized hashmap => chunkmap
struct chunkmap {
	bool g;							 // is global map?
	unsigned int count;	 // count of elements in map
	unsigned int
			logsize;	// log() of the map size ( do 1<<logsize to get full)
	struct chunk* data;
};

// typedef struct sqlite3 sqlite3;
struct sqlite3;

extern struct worldState {
	struct chunkmap load;	 // load + worldgen queue
	struct chunkmap save;
	struct chunkmap update;
	struct chunkmap map;	// chunk map

	unsigned int genmode;	 // worldgen mode
	uint64_t seed;				 // seed
	uint64_t playtime;		 // in seconds

	struct sqlite3* database;

	// gc
	uint64_t gc_limit;

	// flags
	bool is_update_enabled;
	bool is_loading_enabled;
} World;

// specialized murmur hash (was in public domain)
// original : github.com/abrandoned/murmur2/blob/master/MurmurHash2.c
static inline uint32_t murmurhash(uint32_t* data) {
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
	return murmurhash(&value);
}

#define MAPHASH(V) MAPINT(hash_function(V))	 // hash & mapfunc

#include "allocator.h"

void generateChunk(struct chunk*);

void addSaveQueue(struct chunk*);	 // FREES CHUNK AT THE END!!!
void addLoadQueue(struct chunk*);	 // INSERTS CHUNK IN THE TABLE AT THE END!

struct sqlite3_stmt;
struct sqlite3_stmt* create_statement(struct sqlite3* db, const char* sql);
int statement_iterator(struct sqlite3_stmt* stmt);

// HASH
#include "hashmap.h"
