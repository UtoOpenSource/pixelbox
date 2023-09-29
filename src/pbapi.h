/* 
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Pixelbox Public API
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
#include "version.h"

#define CHUNK_WIDTH 16

typedef union packpos { // two int16_t!
	uint32_t pack;
	int16_t axis[2];
} PackedPosition;

typedef struct atoms {
	uint8_t types[CHUNK_WIDTH*CHUNK_WIDTH];
	uint8_t  data[CHUNK_WIDTH*CHUNK_WIDTH];
} Atoms;

typedef struct save_file {
	const char* path;
	uint64_t    time;
	int         mode;
} SaveFile;

// PUBLIC INTERFACE

// global initialization/destruction
int  InitWorld(void);
void FreeWorld(void);

// function below may be called only
// after initialization and before destruction!

void CollectAnything(void); // collect all chunks.
void SetWorldSeed(int64_t); // called ONLY during world creation!

int  OpenWorld(const char* path);
void FlushWorld(void); // save all chunks on the disk. Call after collectAnything()

int  CollectGarbage(void); // collect unused chunks
bool SaveloadTick(); // you must call this every tick for chunks to be loaded/saved!!!
void UpdateWorld(void);

struct chunk* GetWorldChunk(int16_t x, int16_t y); // may fail

#define MODE_READ  0
#define MODE_WRITE 1
struct atoms* GetChunkAtoms(struct chunk*, const bool mode); // +
bool          MarkChunkUpdate(struct chunk* c, uint8_t x, uint8_t y);

// at specified global pixel coords
void SetWorldPixel(int64_t x, int64_t y, uint8_t val, bool mode);
uint8_t GetWorldPixel(int64_t x, int64_t y, bool mode);
struct chunk* MarkWorldUpdate(int64_t x, int64_t y); 

// render
#include "render.h"

// data
bool loadProperty(const char* k, int64_t *v);
bool saveProperty(const char* k, int64_t v);
uint8_t softGenerate(int16_t ox, int16_t oy); // how chunk may look like

// like NULL, but for chunks, and safe to read from
extern struct chunk empty;

bool getWorldInfo(const char* path, uint64_t *time, int *mode);

extern const char* worldgen_modes[];
extern const int worldgen_modes_count;
