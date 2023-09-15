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

static float ptime_old = 0;

void initToolkit();
void freeToolkit();

static void create() {
	WorldRefCreate();
	initBuilder();
	initToolkit();
	ptime_old = GetTime();

	int64_t v;
	prof_begin(PROF_DISK); 
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
	prof_end();
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
	freeToolkit();
	WorldRefDestroy();
}

Camera2D cam = {0};

void setline (int x0, int y0, int x1, int y1);
bool updateToolkit();
void drawToolkit();

static void draw() {
	cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};	
	BeginMode2D(cam); // draw world

	prof_begin(PROF_DRAWWORLD);
	int32_t x0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).x)/ CHUNK_WIDTH - 1;
	int32_t x1 = (GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, cam).x) / CHUNK_WIDTH + 1;
	int32_t y0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).y) / CHUNK_WIDTH - 1;
	int32_t y1 = (GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, cam).y) / CHUNK_WIDTH + 1;

	int i = 0;
	for (int32_t y = y0; y < y1; y++) {
		for (int32_t x = x0; x < x1; x++) {
			struct chunk* c = getWorldChunk(x, y);
			
			if (c == &empty) {
				float rx = ((int32_t)c->pos.axis[0]) * CHUNK_WIDTH;
				float ry = ((int32_t)c->pos.axis[1]) * CHUNK_WIDTH;
				DrawRectangleRec(
					(Rectangle){rx, ry, CHUNK_WIDTH, CHUNK_WIDTH},
					getPixelColor(softGenerate(x, y))
				);
				continue;
			} else renderChunk(c);
			i++;
		}
	}
	flushChunksCache();
	prof_end();

	Vector2 mousepos = GetScreenToWorld2D(GetMousePosition(), cam);

	EndMode2D();
	drawToolkit();
}

static void update() { 
	if (!updateToolkit()) { // interact
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

		if (IsKeyDown(KEY_R)) {
			cam.target = (Vector2){0, 0};
			cam.zoom   = 3;
		}

	}

	prof_begin(PROF_UPDATE);
	updateWorld();
	prof_end(PROF_UPDATE);

	prof_begin(PROF_LOAD_SAVE);
	saveloadTick(); // done in
	prof_end();

	prof_begin(PROF_GC);
	collectGarbage(); // important
	prof_end();

	if (ptime_old + 1 < GetTime()) {
		ptime_old = GetTime();
		World.playtime += 1;
	}
}

struct screen ScrGamePlay = {
	NULL, draw, update, create, destroy
};

