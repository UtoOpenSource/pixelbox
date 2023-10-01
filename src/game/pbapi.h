/*
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Pixelbox Public API
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

/** @file pbapi.h
 *  @brief Pixelbox Global API */ 

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "version.h"
#include <assert.h>

#define CHUNK_WIDTH 16

typedef int32_t ChunkInteger;

typedef union Packpos {	 // two int16_t!
	uint64_t      pack;
	ChunkInteger  axis[2];
} PackPos;

static_assert(sizeof PackPos == sizeof uint64_t);

typedef struct Atoms {
	uint8_t types[CHUNK_WIDTH * CHUNK_WIDTH];
	uint8_t data[CHUNK_WIDTH * CHUNK_WIDTH];
} Atoms;

typedef struct SaveFile {
	const char* path;
	uint64_t time;
	int genmode;
	int chunks_count;
} SaveFile;

typedef struct Chunk Chunk;

// PUBLIC INTERFACE

// global initialization/destruction
int  InitWorld(void);
void FreeWorld(void);

// function below may be called only
// after initialization and before destruction!

void CollectAnything(void);	 // collect all chunks.
void SetWorldSeed(int64_t);	 // called ONLY during world creation!

int  OpenWorld(const char* path);
void FlushWorld(void);	// save all chunks on the disk. Call after
												// collectAnything()

//int  CollectGarbage(void);	 // collect unused chunks
//bool SaveloadTick();	// you must call this every tick for chunks to
											// be loaded/saved!!!
void UpdateWorld(void); // do GC, saveload and world update! call every tick!

Chunk* GetWorldChunk(ChunkInteger x, ChunkInteger y);	// may fail
Chunk* GetWorldLoadedChunk(ChunkInteger x, ChunkInteger y);

#define MODE_READ 0
#define MODE_WRITE 1
Atoms* GetChunkAtoms(Chunk*, const bool mode);	// +
bool   MarkChunkUpdate(Chunk* c, uint8_t x, uint8_t y);

// at specified global pixel coords
// deprecated?
//void SetWorldPixel(int64_t x, int64_t y, uint8_t val, bool mode);
//uint8_t GetWorldPixel(int64_t x, int64_t y, bool mode);
//struct chunk* MarkWorldUpdate(int64_t x, int64_t y);

// render
#include "render.h"

// save/load data
bool LoadProperty(const char* k, int64_t* v);
bool SaveProperty(const char* k, int64_t v);

// how chunk at this coords may look like?
uint8_t PreGenerateChunk(ChunkInteger ox, ChunkInteger oy);

// like NULL, but for chunks, and safe to read from
extern struct Chunk EMPTYCHUNK;
#define NULLCHUNK &EMPTYCHUNK

// must be freed using free()!
SaveFile* GetSaveFileInfo(const char* path);

extern const char* worldgen_modes[];
extern const int   worldgen_modes_count;
