// see pixelbox.h for copyright notice and license.

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define CHUNK_WIDTH 32

union packpos { // two int16_t!
	uint32_t pack;
	int16_t axis[2];
};

/* */
struct chunk {
	struct chunk* next;
	union packpos pos;
	uint8_t  atoms[32*32*2];
	int8_t   usagefactor;
	int8_t    needUpdate; 
}; // we use dirty assumptions about atomic operations here :(

#define MAPLEN 64 // must be pow of 2!

// specialized hashmap => chunkmap
struct chunkmap {
	struct chunk* data[MAPLEN];
};

//typedef struct sqlite3 sqlite3;
struct sqlite3;

extern struct worldState {
	uint64_t seed; // seed
	uint64_t rngstate; // rng (== seed at the beginning)

	bool wIndex; // index of writable array in chunks
	struct chunkmap Map; // chunk map

	struct sqlite3* database; 
} World;

// mark implemented stuff with +
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

int32_t randomNumber(void); // used by main thread ONLY!
float  noise2(float x, float y);
float noise1( float x );
void    setWorldSeed(int64_t); // called ONLY during world creation!

struct chunk* getWorldChunk(int16_t x, int16_t y); // may fail to load/gen
uint64_t getMemoryUsage(); // not accurate

#define MODE_READ  0
#define MODE_WRITE 1
uint8_t* getChunkData(struct chunk*, const bool mode); // +

// PRIVATE INTERFACE MOVED TO IMPPIX.H

