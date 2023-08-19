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

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static void frame() {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		if (SCREEN && SCREEN->draw) SCREEN->draw();

		DrawText("Pixelbox", 0, 0, 10, WHITE);
		EndDrawing();

		if (SCREEN && SCREEN->draw) SCREEN->update();
		collectAssets();

		// screen system
		if (CHANGE) {
			if (SCREEN && SCREEN->destroy) SCREEN->destroy();
			SCREEN = CHANGE;
			CHANGE = NULL;
		}
}

int main() {
	_initAtoms();
	SetTraceLogLevel(LOG_DEBUG);
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "[PixelBox] : amazing description");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	GuiLoadStyleDark();
	initAssetSystem();

	SetRootScreen(&ScrMainMenu);
	//initWorld();

	#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(frame, 0, 1);
	#else
	while (!WindowShouldClose()) { 
		frame();
	};
	#endif

	// free screen
	if (SCREEN && SCREEN->destroy) SCREEN->destroy();

	freeAssetSystem();
	CloseWindow();
	return 0;
}
