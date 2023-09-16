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

#include "implix.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t* getChunkData(struct chunk* c, const bool mode) {
	return c->atoms + (mode == c->wIndex ? CHUNK_WIDTH*CHUNK_WIDTH : 0);
}

#include <string.h>
#include <math.h>

#include "profiler.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

const char* world_modes[] = {
	"Normal",
	"Flat",
	"Sponge"
};

const int world_modes_count = 3;

static uint8_t gen_normal(int32_t ax, int32_t ay) {
	float v;
	
	// "cave"
	float c = noise2(ax/64.0, ay/64.0) + 0.1;
	c = c * sinf(noise2(ax/128.0, ay/128.0));
	c = c - MAX(noise2(ax/512.0, ay/512.0), 0.2) 
		* noise2(ax/2048.0, ay/2048.0);

	v = c;
	if (v > 0.09) {
		c *= 0.5;
		v = noise2(ax/124.0, ay/124.0) + c;
		v = v > 1.0 ? v : 1.0 - v;
		v += 0.02;
	} else v = 0; 

	return v*255;
}

static uint8_t gen_sponge(int32_t ax, int32_t ay) {
	long int pow = 1;
	int v = 0;

	for (int deep = 0; deep < 10; deep++) {
		if ((ax/pow)%3 == 1 && (ay/pow)%3==1) goto skip_set;
		pow = pow * 3;
	}
	v = noise2(ax/512.0, ay/512.0) * 128 + 128;
	skip_set:
	return v;
}

static uint8_t gen_flat(int32_t ax, int32_t ay) {
	int v = ((ax & 1023) == 64) + ((ay & 1023) == 64);
	return v ? (v+1)<<2 : 0;
}

static uint8_t (*generators[])(int32_t x, int32_t y) = {
	gen_normal,
	gen_flat,
	gen_sponge,
	NULL
};

uint8_t softGenerate(int16_t ox, int16_t oy) {
	int x = (int32_t)ox*CHUNK_WIDTH;
	int y = (int32_t)oy*CHUNK_WIDTH;

	uint8_t (*generator)(int32_t x, int32_t y);
	if (World.mode < 0) generator = generators[1];
	else generator = World.mode < 3 ? generators[World.mode] : generators[1];

	const int z = CHUNK_WIDTH-1;
	int v = 0;
	v += generator(x + 0, y + 0);
	v += generator(x + z, y + 0);
	v += generator(x + z, y + z);
	v += generator(x + 0, y + z);
	v /= 4;
	return v;
}

void generateChunk(struct chunk* c) {
	prof_begin(PROF_GENERATOR);

	uint8_t* data = getChunkData(c, MODE_READ);
	uint8_t (*generator)(int32_t x, int32_t y);

	if (World.mode < 0) generator = generators[1];
	else generator = World.mode < 3 ? generators[World.mode] : generators[1];

	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {	
			data[x + y * CHUNK_WIDTH] = generator(
				(int32_t)c->pos.axis[0]*CHUNK_WIDTH + x,
				(int32_t)c->pos.axis[1]*CHUNK_WIDTH + y
			);
		}
	}

	prof_end();
}

