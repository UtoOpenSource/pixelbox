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

static void generateChunkNormal(struct chunk* c);
static void generateChunkFlat(struct chunk* c);
static void generateChunkSponge(struct chunk* c);

void generateChunk(struct chunk* c) {
	switch (World.mode) {
		case 0 : generateChunkNormal(c); break;
		case 1 : generateChunkFlat(c); break;
		case 2 : generateChunkSponge(c); break;
		default: break;
	}
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void generateChunkNormal(struct chunk* c) {
	uint8_t* data = getChunkData(c, MODE_READ);
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {
			int ax = (x + (int)c->pos.axis[0]*CHUNK_WIDTH);
			int ay = (y + (int)c->pos.axis[1]*CHUNK_WIDTH);
			float v;

			// "cave"
			float c = noise2(ax/64.0, ay/64.0) + 0.1;
			c = c * sinf(noise2(ax/128.0, ay/128.0));
			c = c - MAX(noise2(ax/512.0, ay/512.0), 0.2) 
				* noise2(ax/2048.0, ay/2048.0);

			v = c;
			if (v > 0.1) {
				c *= 0.5;
				v = noise2(ax/124.0, ay/124.0) + c;
				v = v > 1.0 ? v : 1.0 - v;
				v += 0.02;
			} else v = 0; 
			data[x + y * CHUNK_WIDTH] = v*255;
		}
	}
}

static void generateChunkSponge(struct chunk* c) {
	uint8_t* data = getChunkData(c, MODE_READ);
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {	
			int ax = (x + (int)c->pos.axis[0]*CHUNK_WIDTH);
			int ay = (y + (int)c->pos.axis[1]*CHUNK_WIDTH);
			long int pow = 1;
			int v = 0;

			for (int deep = 0; deep < 10; deep++) {
				if ((ax/pow)%3 == 1 && (ay/pow)%3==1) goto skip_set;
				pow = pow * 3;
			}
			v = noise2(ax/512.0, ay/512.0) * 128 + 128;
			skip_set:
			data[x + y * CHUNK_WIDTH] = v;
		}
	}
}

static void generateChunkFlat(struct chunk* c) {
	uint8_t* data = getChunkData(c, MODE_READ);
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {
			int ax = (x + (int)c->pos.axis[0]*CHUNK_WIDTH);
			int ay = (y + (int)c->pos.axis[1]*CHUNK_WIDTH);
			int v;
	
			v = ((ax & 1023) == 64) + ((ay & 1023) == 64);
			data[x + y * CHUNK_WIDTH] = v ? (v+1)<<2 : 0;
		}
	}
}

