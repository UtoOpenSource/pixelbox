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

static int refcnt = 0;

void WorldRefCreate() {
	if (!refcnt) initWorld();
	refcnt++;
}

void WorldRefDestroy() {
	if (!refcnt) return;
	refcnt--;
	if (!refcnt) freeWorld();
}

#include <limits.h>

static void create() {
	WorldRefCreate();
	initBuilder();

	int64_t v;
	if (loadProperty("zoom", &v)) {
		cam.zoom = v / 7000.0;
		if (cam.zoom < 0.5) cam.zoom = 0.5;
		if (cam.zoom > 10) cam.zoom = 10;
	} else cam.zoom = 3;

	int64_t x, y;
	if (loadProperty("camx", &x) && loadProperty("camy", &y)) {
		cam.target = (Vector2){x/5.0, y/5.0};
	} else cam.target = (Vector2){120/2, 120/2};
	cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};
}

static void destroy() {
	int64_t v;

	v = cam.zoom * 7000;
	saveProperty("zoom", v);

	v = cam.target.x * 5;
	saveProperty("camx", v);

	v = cam.target.y * 5;
	saveProperty("camy", v);

	freeBuilder();
	WorldRefDestroy();
}

Camera2D cam = {0};
int color_gradient = -1;
int color_material = -1;

static uint8_t getcolor() {
	uint8_t color = 0;
	color = color_material >= 0 ? color_material : randomNumber();
	color <<= 2;
	color |= color_gradient >= 0 ? color_gradient & 3 : randomNumber() & 3;
	return color;
}

void setline (int x0, int y0, int x1, int y1) {
	int dx = ABS(x1 - x0);
	int dy = ABS(y1 - y0) * -1;
	int stepx = x0 < x1 ? 1 : -1;
	int stepy = y0 < y1 ? 1 : -1;
	
	int err = dx + dy, e2 = 0;
	int limit = 0;
	
	while (limit < 255) {
		limit++;
		setWorldPixel(x0, y0, getcolor(), MODE_READ);
		markWorldUpdate(x0, y0);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) {
			err = err + dy;
			x0  = x0 + stepx;
		}
		if (e2 <= dx) {
			err = err + dx;
			y0  = y0 + stepy;
		}
	}
}

static void drawGUI();

static void draw() {
	cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};	
	BeginMode2D(cam); // draw world

	int32_t x0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).x)/ CHUNK_WIDTH - 1;
	int32_t x1 = (GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, cam).x) / CHUNK_WIDTH + 1;
	int32_t y0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).y) / CHUNK_WIDTH - 1;
	int32_t y1 = (GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, cam).y) / CHUNK_WIDTH + 1;

	int i = 0;
	for (int32_t y = y0; y < y1; y++) {
		for (int32_t x = x0; x < x1; x++) {
			renderChunk(getWorldChunk(x, y));
			i++;
		}
	}
	flushChunksCache();

	Vector2 mousepos = GetScreenToWorld2D(GetMousePosition(), cam);

	EndMode2D();
	drawGUI();
}

// GUI

Rectangle winrec = (Rectangle){
	0, 100,
	400, 110
};

static const char* tabs[] = {
	"world",
	"pallete",
	"stats",
	NULL
};

static int active_tab = 1;
static int allowupdate = 0;
static int window_hidden = 0;

static void drawGUI() {
	GuiPanel(winrec, TextFormat("[PIXELBOX v.%1.1f] (%i FPS)", 1.0, (int)GetFPS()));
	Rectangle rec = (Rectangle) { // Hide button rectangle
		winrec.x + winrec.width - 20, winrec.y + 2,
		20, 20
	};

	int icon_kind = window_hidden ? ICON_ARROW_UP : ICON_ARROW_DOWN;

	if (GuiButton(rec, GuiIconText(icon_kind, ""))) {
		window_hidden = !window_hidden;
	}

	if (window_hidden) return; // :)
	
	// content rectangle
	rec = (Rectangle) {
		winrec.x, winrec.y + 25,
		winrec.width, winrec.height - 25
	};

	int old = rec.height;
	rec.height = 20;
	GuiTabBarEx(rec, 80, false, tabs, 3, &active_tab);
	rec.y += 25;
	rec.height = old - 25;
	rec.x += 5;
	rec.width -= 5;

	Rectangle item = (Rectangle){
		rec.x, rec.y,
		rec.width/2, 25
	};

	switch(active_tab) {
		case 0 : {// world
			allowupdate = GuiToggle(item, "Enable Physic", allowupdate);
		}; break;
		case 1 : {// pallete 
			int new = GuiToggle(item, "random", color_material < 0);
			if (new != (color_material < 0)) { 
				if (new) color_material = -1; 
				else color_material = 0;
			}
		}; break;
		case 2 : {// stats
			GuiLine(item, "stats");
			item.y += item.height;

			GuiLine(item, TextFormat("%i:%i[%i:%i]", 
				(int)cam.target.x,
				(int)cam.target.y,
				(unsigned int)cam.target.x/CHUNK_WIDTH,
				(unsigned int)cam.target.y/CHUNK_WIDTH
				)
			);
		}; break;
		default :

		break;
	}
}

static void update() { 
	winrec = (Rectangle){
		0, GetScreenHeight() - (window_hidden ? 25 : 200),
		400, (window_hidden ? 25 : 205)
	};
	GuiLock();
	if (CheckCollisionPointRec(GetMousePosition(), winrec)) {
		// gui
		GuiUnlock();
	} else { // interact
		if (IsMouseButtonDown(0)) {
			Vector2 md = GetMouseDelta();
			cam.target.x -= md.x /cam.zoom;
			cam.target.y -= md.y /cam.zoom;
		}

		cam.zoom += GetMouseWheelMove() * 0.15 * cam.zoom;
		if (cam.zoom < 0.01) cam.zoom = 0.01;
		if (cam.zoom > 100) cam.zoom = 100;

		// input
		if (IsMouseButtonDown(1)) {
			Vector2 dpos = GetMouseDelta();
			Vector2 cpos = GetMousePosition();
			dpos.x = cpos.x - dpos.x;
			dpos.y = cpos.y - dpos.y;
			dpos = GetScreenToWorld2D(dpos, cam); 
			cpos = GetScreenToWorld2D(cpos, cam); 
			setline(cpos.x, cpos.y, dpos.x, dpos.y);
		}

		if (IsKeyDown(KEY_U)) {
			updateWorld();
		}

		if (IsKeyDown(KEY_R)) {
			cam.target = (Vector2){0, 0};
		}

	}

	if (allowupdate) updateWorld();

	collectGarbage(); // important
}

struct screen ScrGamePlay = {
	NULL, draw, update, create, destroy
};

