// see pixelbox.h for copyright notice and license.

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define CHUNK_WIDTH 32

union packpos { // two int16_t!
	uint32_t pack;
	int16_t axis[2];
};

/* */
struct chunk {
	uint8_t  atomsA[32*32];
	uint8_t  atomsB[32*32];
	int8_t    usagefactor;
	union packpos pos;
	struct chunk* next;
};

#define MAPLEN 64 // must be pow of 2!
#define MAPINT(V) (V & (MAPLEN-1))
#define MAPHASH(V) MAPINT(V ^ (V*234975434)) // hash & mapfunc

extern struct worldState {
	uint64_t seed; // seed
	uint64_t rngstate; // rng (== seed at the beginning)

	bool wIndex; // index of writable array in chunks
	struct chunk* GameMap[MAPLEN]; // processing and interactable chunks
	

	struct chunk* LoadMap[MAPLEN]; // chunks that's loading now
	struct chunk* SaveMap[MAPLEN]; // chunks that's saving now
} World;

int  initWorld(void);
void freeWorld(void);

int  loadWorld(const char* path);

int32_t randomNumber(void);
void    setSeed(int64_t);

void collectAnything(void);
int collectGarbage(void);


