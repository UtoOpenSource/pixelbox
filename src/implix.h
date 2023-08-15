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

void updateChunk(struct chunk* c);
