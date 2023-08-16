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
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static void create() {

}

static void destroy() {

}

// world selection dialog
static void draw() {
	GuiEnable();

	Rectangle rec = GuiMenuWindow("Pixelbox");
	int orig_h = rec.height;
	rec.height = 25;
	GuiLine(rec, "PIXELBOX");

	rec.y += rec.height + 5;
	if (GuiButton(rec, "Open World")) {
		SetNextScreen(&ScrWorldList);
	}
	
	rec.y += rec.height + 5;
	if (GuiButton(rec, "Create New World")) {
		//SetNextScreen(&ScrNewWorld);
	}

	rec.y += (rec.height + 5)*2;
	GuiLine(rec, "blablabla");

	rec.y += rec.height + 5;
	GuiLine(rec, "Copyright (C) UtoECat 2023");

	rec.y += rec.height + 5;
	if (GuiButton(rec, "License")) {
		SetNextScreen(&ScrLicense);
	}
}

static void update() {

}

struct screen ScrMainMenu = {
	NULL, draw, update, create, destroy
};

