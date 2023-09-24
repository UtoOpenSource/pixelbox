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
#include "profiler.h"

static Rectangle dwinrec = (Rectangle){
	0, 100,
	400, 110
};

void initDToolkit() {
	dwinrec = (Rectangle){
		GetScreenWidth() - 470, 0,
		450, 20
	};
}

void freeDToolkit() {

}

static const char* tabs[] = {
	"profiler",
	"hashtable",
	"rendering",
	"allocator",
	"control",
	NULL
};

static int active_tab = 1;
static int active_hash = 0;
static int window_hidden = 1;

void drawProfiler(Rectangle rec);
void debugRender(Rectangle rec);
void debugAllocator(Rectangle rec);

#include "implix.h"

static bool collides(struct chunk* o, int32_t x, int32_t y, int32_t x2, int32_t y2) {
	return (
		(int32_t)o->pos.axis[0] <= x2 &&
    (int32_t)o->pos.axis[0] >= x  &&
    (int32_t)o->pos.axis[1] <= y2 &&
    (int32_t)o->pos.axis[1] >= y
	);
}

#define swap(a, b) {do {int t = a; a = b; b = t;} while(0);}

void debugHash(Rectangle rec) {
	int64_t x0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).x)/ CHUNK_WIDTH - 1;
	int64_t x1 = (GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, cam).x) / CHUNK_WIDTH;
	int64_t y0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).y) / CHUNK_WIDTH - 1;
	int64_t y1 = (GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, cam).y) / CHUNK_WIDTH;
	if (x1 < x0) swap(x1, x0);
	if (y1 < y0) swap(y1, y0);	


	Rectangle item = {rec.x, rec.y, 50, 20};
	active_hash = GuiToggleGroup(item, "Map;Load;Save;Update", active_hash);

	rec.y += 25;

	struct chunkmap* m = NULL;
	if (active_hash == 0) m = &World.map;
	else if (active_hash == 1) m = &World.load;
	else if (active_hash == 2) m = &World.save;
	else m = &World.update;

	for (int i = 0; i < MAPLEN; i++) {
		struct chunk *o = m->data[i];
		int j = 0;
		DrawPixel(rec.x + i, rec.y - 1, YELLOW);
		while (o) {
			DrawPixel(rec.x + i, rec.y + j,Fade(
				collides(o, x0, y0, x1, y1) ? MAGENTA : BLUE, o->usagefactor/(float)CHUNK_USAGE_VALUE));
			if (m->g) o = o->next;
			else o = o->next2;
			j++;	
		}
	}
}

static void controlTab(Rectangle rec) {
	Rectangle item = (Rectangle){
		rec.x, rec.y,
		rec.width-25, 10
	};
}

#include "version.h"

bool CheckCurrentScreen(struct screen* CURR);
bool safeWorld() {
	return CheckCurrentScreen(&ScrGamePlay);
}

void drawDToolkit() {
	GuiLock();
	if (CheckCollisionPointRec(GetMousePosition(), dwinrec)) {
		GuiUnlock();
	}

	GuiPanel(dwinrec, "DEBUG");
	Rectangle rec = (Rectangle) { // Hide button rectangle
		dwinrec.x + dwinrec.width - 20, dwinrec.y + 2,
		20, 20
	};

	int icon_kind = window_hidden ? ICON_ARROW_UP : ICON_ARROW_DOWN;

	if (GuiButton(rec, GuiIconText(icon_kind, ""))) {
		window_hidden = !window_hidden;
	}

	if (window_hidden) {
		GuiUnlock();
		return; // :)
	}

	// content rectangle
	rec = (Rectangle) {
		dwinrec.x, dwinrec.y + 25,
		dwinrec.width, dwinrec.height - 25
	};

	int old = rec.height;
	rec.height = 20;
	GuiTabBarEx(rec, 80, false, tabs, 5, &active_tab);
	rec.y += 25;
	rec.height = old - 25;
	rec.x += 5;
	rec.width -= 5;

	switch(active_tab) {
		case 0 :
			drawProfiler(rec);
		break;
		case 1 :
			if (safeWorld()) debugHash(rec);
		break;
		case 2:
			if (safeWorld()) debugRender(rec);
		break;
		case 3 :
			debugAllocator(rec);
		break;
		case 4:
			controlTab(rec);
		break;
		default :	
		break;
	}
	GuiUnlock();
}

static int move = 0;

bool updateDToolkit() { 
	/*dwinrec = (Rectangle){
		GetScreenWidth() - 500, 0,
		450, (window_hidden ? 20 : 250)
	}; */

	Rectangle winbarrec = (Rectangle){
		dwinrec.x, dwinrec.y,
		400, 20
	};

	dwinrec.height = (window_hidden ? 20 : 250);

	if (move && IsMouseButtonDown(0)) {
		Vector2 delta = GetMouseDelta();
		dwinrec.x += delta.x;
		dwinrec.y += delta.y;
	} else move = 0;

	if (dwinrec.x < 0) dwinrec.x = 0;
	if (dwinrec.y < 0) dwinrec.y = 0;
	if (dwinrec.x + dwinrec.width > GetScreenWidth())
		dwinrec.x = GetScreenWidth() - dwinrec.width;
	if (dwinrec.y + dwinrec.height > GetScreenHeight())
		dwinrec.y = GetScreenHeight() - dwinrec.height;

	if (CheckCollisionPointRec(GetMousePosition(), dwinrec)) {
		if (CheckCollisionPointRec(GetMousePosition(), winbarrec) &&
				IsMouseButtonPressed(0)
		) {
			move = 1;
		}
		return true;
	}
	return false;
}
