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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "profiler.h"
#include "random.h"
#include "raygui.h"
#include "settings.h"
#include "screen.h"

namespace engine {

class UninitHook {
	public:
	virtual void destroy() = 0;
};

extern bool verbose;
extern ::conf::Manager config;

void stop();

void init(bool gui, const char* const* names);
void uninit();

void join(int (*cb)(double, bool));

double getTime();
float  deltaTime();

void   subscribeUninit(UninitHook*);

};

