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
#include <stdlib.h>

static struct screen *SCREEN = &ScrNull, *CHANGE = NULL;

void SetNextScreen(struct screen* NEW) {
	if (NEW == SCREEN || CHANGE) return;
	if (NEW && NEW->create) NEW->create();
	CHANGE = NEW;
	if (NEW) NEW->back = SCREEN;
}

void SetRootScreen(struct screen* NEW) {
	if (NEW == SCREEN || CHANGE) return;
	if (NEW && NEW->create) NEW->create();
	CHANGE = NEW;
	if (NEW) NEW->back = NULL;
}

int SetPrevScreen(struct screen* FALLBACK) {
	struct screen* NEW = (SCREEN && SCREEN->back) ? SCREEN->back : FALLBACK;
	if (NEW == SCREEN || CHANGE) return -1;
	if (NEW && NEW->create) NEW->create();
	CHANGE = NEW;
	return 0;
}

#include "raygui.h"
Rectangle GuiMenuWindow(const char* title) {
	Rectangle rec = {
		GetScreenWidth()/6, GetScreenHeight()/6,
		GetScreenWidth()/1.5, GetScreenHeight()/1.5
	};
	GuiPanel(rec, title);
	rec.x += 5;
	rec.width -= 10;
	rec.y += 25;
	rec.height -= 23;	

	if (SCREEN && SCREEN->back) {
		Rectangle rec2 = {10, 10, 60, 25};
		if (GuiButton(rec2, "< Back")) {
			SetPrevScreen(NULL);
		}
	}
	return rec;
}

#include "assets.h"

void GuiAssetTexture(Rectangle rec, AssetID id) {
	Texture tex = GetTextureAsset(id);

	DrawTexturePro(tex, (Rectangle){0, 0, tex.width, tex.height}, 
		rec, (Vector2){0, 0}, 0, WHITE);
}

void _initAtoms();
void GuiLoadStyleDark();

#include "profiler.h"

bool game_working = true;

static void frame(bool should_close) {
		prof_begin(PROF_FINDRAW);
		BeginDrawing();
		ClearBackground(RAYWHITE);
		prof_end(PROF_FINDRAW);

		prof_begin(PROF_DRAW);
		if (SCREEN && SCREEN->draw) SCREEN->draw();
		DrawText("Pixelbox", 0, 0, 10, WHITE);
		prof_end();

		prof_begin(PROF_UPDATE);
		if (SCREEN && SCREEN->draw) SCREEN->update();
		prof_end();
	
		prof_begin(PROF_GC);
		collectAssets();
		prof_end(); 

		prof_begin(PROF_FINDRAW);
		EndDrawing();
		prof_end();

		// screen system
		prof_begin(PROF_INIT_FREE);
		if (CHANGE) {
			if (SCREEN && SCREEN->destroy) SCREEN->destroy();
			SCREEN = CHANGE;
			CHANGE = NULL;
		}
		if (should_close) {
			if (SCREEN && SCREEN->onclose) SCREEN->onclose();
			else game_working = false;
		}
		prof_end();
}

Color getPixelColor(uint8_t val) {
	float kind = (val & 3)/6.0;
	int type = (val >> 2) & 63;
	float r  = (type & 3) + kind;
	float g  = ((type >> 2) & 3) + kind;
	float b  = ((type >> 4) & 3) + kind;
	Color color = {r/4.0*255, g/4.0*255, b/4.0*255, 255};
	return color;
}

int main() {
	prof_begin(PROF_INIT_FREE);
	_initAtoms();
	SetTraceLogLevel(LOG_DEBUG);
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "[PixelBox] : loading");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	GuiLoadStyleDark();
	initAssetSystem();
	SetRootScreen(&ScrMainMenu);
	prof_end();

	while (game_working) { //!) { 
		prof_begin(PROF_GAMETICK);
		frame(WindowShouldClose());
		prof_end();

		prof_step();
	};
	
	// free screen
	if (SCREEN && SCREEN->destroy) SCREEN->destroy();

	freeAssetSystem();
	CloseWindow();
	return 0;
}
