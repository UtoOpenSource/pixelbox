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

#include "raygui.h"
#include "pixel.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

static bool status = false;
static char seed     [64] = {0};
static char filename [64] = ":null:";
static int  mode          = 0;
static int  input = 0, input2 = 0;

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#endif

static void create() {
	WorldRefCreate();
	#if _WIN32
	_mkdir(".\\saves");
	#else
	mkdir("./saves", 0777);
	#endif
	snprintf(seed, 64, "%li", time(NULL)*clock());
}

static void destroy() {
	WorldRefDestroy();
}

void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*s == ' ') {
            s++;
        }
    } while (*d++ = *s++);
}

static void draw() {
	Rectangle rec = GuiMenuWindow("Create New World");
	rec.height = 25;
	rec.width /= 2;

	GuiLine(rec, "World Name");

	rec.y += 30;
	GuiLine(rec, "Seed ");

	rec.y += 30;
	GuiLine(rec, "Terrain");
	
	rec.y -= 60;
	rec.x += rec.width;
	if (GuiTextBox(rec, filename, 63, input)) input = !input;

	rec.y += 30;
	if (GuiTextBox(rec, seed, 63, input2)) input2 = !input2;

	rec.y += 30;
	mode = GuiComboBox(rec, "Normal;Flat;Sponge", mode);

	rec.y += 80;
	rec.x -= rec.width - 2;
	if (GuiButton(rec, "Abort")) {
		SetPrevScreen(&ScrMainMenu);
	};

	rec.x += rec.width + 2;
	if (GuiButton(rec, "Done")) {
		remove_spaces(filename);
		const char* src;

		if (strcmp(filename, ":memory:") == 0)
			src = ":memory:";
		else if (strcmp(filename, ":null:") == 0)
			goto skip_load;
		else
			src = TextFormat("./saves/%s.db", filename);
		SetWindowTitle(TextFormat("[pixelbox] : %s", src));
		openWorld(src);

skip_load:
		uint64_t seedr = 0;
		sscanf(seed, "%li", &seedr);
		setWorldSeed(seedr);

		World.mode = mode;

		SetRootScreen(&ScrGamePlay);
	}
}

static void update() {

}

struct screen ScrNewWorld = {
	NULL, draw, update, create, destroy
};

