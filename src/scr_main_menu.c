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

#include "assets.h"
static const char* title_text = 
"there is one\nsusposter\namulgus"
;

const char* splash = "indev!";

// world selection dialog
static void draw() {
	GuiEnable();

	Rectangle rec = GuiMenuWindow("Pixelbox");

	Rectangle item = {rec.x, rec.y, 100, 100};
	item.x += rec.width/2 - 50 - 50;
	GuiAssetTexture(item, LookupAssetID("icon.png"));
	item.x += 105;
	item.width = 90;
	GuiLabel(item, TextFormat("Pixelbox v.%1.1f\n%s",
		(float) PBOX_VERSION, title_text));
	
	rec.y += 100 + 5;
	rec.height -= 100 + 5;
	item = (Rectangle){rec.x, rec.y, rec.width, 25};

	if (GuiButton(item, "Open World")) {
		SetNextScreen(&ScrWorldList);
	}
	
	item.y += item.height + 5;
	if (GuiButton(item, "Create New World")) {
		SetNextScreen(&ScrNewWorld);
	}

	item.y += item.height + 5;
	Rectangle old = item;
	item.width = item.width / 2 - 5;
	
	GuiButton(item, "Settings");
	item.x += item.width + 5;
	GuiButton(item, "Exit program");

	item = old;

	item.y += item.height + 5;
	GuiLabel(item, "blablabla");

	item.y = rec.y + rec.height - (5 + 25);
	GuiLabel(item, "Copyright (C) UtoECat 2023");

	item.x += item.width / 3 * 2;
	item.width /= 3;
	item.width += 0.5;
	if (GuiButton(item, "License")) {
		SetNextScreen(&ScrLicense);
	}
}

static void update() {

}

struct screen ScrMainMenu = {
	NULL, draw, update, create, destroy
};

