/* 
 * Pixelbox
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

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define CHUNK_WIDTH 16

union packpos { // two int16_t!
	uint32_t pack;
	int16_t axis[2];
};

/* */
struct chunk {
	struct chunk *next, *next2; // next2 for minimap!
	union packpos pos;
	uint8_t	atoms[CHUNK_WIDTH*CHUNK_WIDTH*2];
	int8_t	usagefactor; // GC
	int8_t	wasUpdated : 1;
	int8_t  is_changed : 1;
	bool		wIndex; 
};

#define MAPLEN 128 // must be pow of 2!
#define MINILEN 128 // must be pow of 2!

// specialized hashmap => chunkmap
struct chunkmap {
	bool g; // is global map?
	struct chunk* data[MAPLEN];
};

//typedef struct sqlite3 sqlite3;
struct sqlite3;

extern struct worldState {
	uint64_t rngstate; // rng (== seed at the beginning)

	struct chunkmap load; // load + worldgen queue
	struct chunkmap save;
	struct chunkmap update;
	struct chunkmap map; // chunk map

	int mode; // worldgen mode
	uint64_t seed; // seed
	uint64_t playtime;
	
	struct sqlite3* database; 

	// flags
	bool is_update_enabled;
} World;


struct save_file {
	const char* path;
	uint64_t    time;
	int         mode;
};

// PUBLIC INTERFACE

// global initialization/destruction
int  initWorld(void);
void freeWorld(void);

// function below may be called only
// after initialization and before destruction!

int  openWorld(const char* path);
void flushWorld(void); // save all unsaved chunks. Call after collectAnything()

void collectAnything(void); // collect all chunks.
int collectGarbage(void); // collect unused chunks

bool saveloadTick(); // you must call this every tick for chunks to be loaded/saved!!!

int32_t randomNumber(void); // used by main thread ONLY!
float  noise2(float x, float y);
float  noise1( float x );
void    setWorldSeed(int64_t); // called ONLY during world creation!

void updateWorld(void);

struct chunk* getWorldChunk(int16_t x, int16_t y); // may fail to load/gen
uint64_t getMemoryUsage(); // not accurate

#define MODE_READ  0
#define MODE_WRITE 1
uint8_t* getChunkData(struct chunk*, const bool mode); // +

// at specified global pixel coords
void setWorldPixel(int64_t x, int64_t y, uint8_t val, bool mode);
uint8_t getWorldPixel(int64_t x, int64_t y, bool mode);
struct chunk* markWorldUpdate(int64_t x, int64_t y); 

// data
bool loadProperty(const char* k, int64_t *v);
bool saveProperty(const char* k, int64_t v);
uint8_t softGenerate(int16_t ox, int16_t oy); // how chunk may look like

extern struct chunk empty;

bool getWorldInfo(const char* path, uint64_t *time, int *mode);

extern const char* world_modes[];
extern const int world_modes_count;
