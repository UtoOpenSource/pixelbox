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

#include <assert.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine.h"
#include "pbapi.h"

const char* getSplash();
void rngSplash();

static double old_time;

static void create() {
	old_time = GetTime() + 3;
	rngSplash();
	SetWindowTitle(TextFormat("[pixelbox] : %s", getSplash()));
}

static void destroy() {}

#include "assets.h"
static const char* title_text = "an infinite 2D\nsandbox game!\n";

const char* splash = "indev!";

// world selection dialog
static void draw() {
	GuiEnable();
	// GuiUnlock();
	// fprintf(stderr, "OK\n");

	Rectangle rec = GuiMenuWindow("Pixelbox");

	Rectangle item = {rec.x, rec.y, 100, 100};
	item.x += rec.width / 2 - 50 - 70;
	GuiAssetTexture(item, LookupAssetID("assets/icon.png"));
	item.x += 105;
	item.width = 140;
	GuiLabel(item, TextFormat("Pixelbox v.%s\n%s%s", PBOX_VERSION,
														title_text, getSplash()));

	rec.y += 100 + 5;
	rec.height -= 100 + 5;
	item = (Rectangle){rec.x, rec.y, rec.width, 25};

	rec.y += 10;
	rec.height -= 12;

	if (GuiButton(item, "Open World")) {
		SetNextScreen(&ScrWorldList);
	}

	item.y += item.height + 5;
	if (GuiButton(item, "Create New World")) {
		SetNextScreen(&ScrNewWorld);
	}

	item.y += item.height + 5;
	Rectangle old = item;
	item.width = item.width / 2 - 2;

	if (GuiButton(item, "Settings")) {
		SetNextScreen(&ScrSettings);
	};

	item.x += item.width + 4;
	if (GuiButton(item, "Exit program")) {
		_StopEngine();
	};

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

static void update() {}

static void onclose() {
	if (GetTime() > old_time) _StopEngine();	// can ESC
	fprintf(stderr, "ESC\n");
}

struct screen ScrMainMenu = {NULL,	 draw,		update,
														 create, destroy, onclose};
