/*
 * This file is a part of Pixelbox - Infinite 2D sandbox game
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
#pragma once
#include <stdint.h>

class RNG {
	private:
	uint64_t state = 0;
	uint64_t next();
	public:
	int32_t get();
	double  getn(); // get normalized
	inline int32_t operator()(void) {
		return get();
	}	
	void    seed(uint64_t);
};

int32_t randomNumber(void);
void randomizeNoise(uint64_t seed);

float grad1(int hash, float x);
float grad2(int hash, float x, float y);

float noise1(float x);
float pnoise1(float x, int px);
float noise2(float x, float y);
float pnoise2(float x, float y, int px, int py);
