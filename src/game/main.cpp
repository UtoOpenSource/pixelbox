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

#include "engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>

int addformat(std::string& buff, const char* fmt, ...);

std::string getConfigDir(const char* appname);
std::string getCacheDir(const char* appname);
std::string getDataDir(const char* appname);

static const char* const names[] = {
	"tick", "disk_io", "gc", "draw", // theese four are MANDATORY for main thread! (Used by the engine!)
	NULL
};

namespace screen {
	extern Base* Debug;
	extern Base* Test;
};

screen::Manager screens;

static int cb(double dt, bool should) {
	screens.draw();
	screens.update(dt);
	return should;
}

int main(int argc, char** argv) {
	bool is_client = true;
	const char* world_name = nullptr; 
	::engine::verbose = 1;

	engine::init(1, names);
	screens.setDebug(screen::Debug);
	screens.setRoot(screen::Test);

	engine::join(cb);

	// important
	screens.release();
	engine::uninit();
}
