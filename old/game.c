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

#include <raylib.h>
#include "pixel.h"
#include "game.h"
#include <stdio.h>

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

void _initAtoms();

#include <stdlib.h>
char* world_db_path = NULL;

int main() {
	_initAtoms();
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "[PixelBox] : amazing description");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	GuiLoadStyleDark();

	if (IntroDialog()) { // interrupted
		CloseWindow();
		return 0;
	}

	initBuilder();
	initWorld();

	if (!world_db_path) { // no path? => new world!
		if (CreationDialog()) {
			freeWorld();
			freeBuilder();
			CloseWindow();
			return 0;
		}
	} else openWorld(world_db_path);

	cam.zoom = 3;
	cam.rotation = 0;
	cam.target = (Vector2){120/2, 120/2};
	cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};

	while (!WindowShouldClose()) { 
		cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};
		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode2D(cam); // draw world

		int32_t x0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).x) / CHUNK_WIDTH - 1;
		int32_t x1 = (GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, cam).x) / CHUNK_WIDTH + 1;
		int32_t y0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).y) / CHUNK_WIDTH - 1;
		int32_t y1 = (GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, cam).y) / CHUNK_WIDTH + 1;

		for (int32_t y = y0; y < y1; y++) {
			for (int32_t x = x0; x < x1; x++) {
				renderChunk(getWorldChunk(x, y));
			}
		}
		flushChunksCache();

		Vector2 mousepos = GetScreenToWorld2D(GetMousePosition(), cam);

		EndMode2D();
		UpdateGUI(); // draw gui!
		Vector2 campos = GetScreenToWorld2D((Vector2){0, 0}, cam);
		DrawText(TextFormat(
				"PIXELBOX %i FPS, %i FRAMETIME\n[NORMAL]:[CHUNK]\n\
CAM[%i, %i]\nCUR[%i, %i]:[%i, %i]\n",
				(int)GetFPS(),
				(int)(GetFrameTime()*1000),
				(int)campos.x,
				(int)campos.y,
				(int)mousepos.x,
				(int)mousepos.y,
				(int)mousepos.x/CHUNK_WIDTH,
				(int)mousepos.y/CHUNK_WIDTH
			), 0, 0, 26, WHITE);
		EndDrawing();

		collectGarbage();

	};

	freeWorld();
	free(world_db_path);
	freeBuilder();
	CloseWindow();
	return 0;
}
