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
#include "engine.h"
#include "screen.h"
#include <stdio.h>
#include <stdlib.h>

static bool is_game_working = true;

void GuiLoadStyleDark();
void initDToolkit();
void freeDToolkit();
void drawDToolkit();
bool updateDToolkit();

void _StopEngine() {
	is_game_working = false;
}

void _InitEngine() {
	prof_register_thread(); // init profiler
	prof_begin(PROF_INIT_FREE);
	reloadSettings(); // load settings
	
	// raylib - set log and vsync/fps limit
	SetTraceLogLevel(LOG_INFO);
	if (conf_vsync) { 
		SetConfigFlags(FLAG_VSYNC_HINT);
	} else SetTargetFPS(conf_max_fps);

	// init raylib window
	InitWindow(conf_win_width, conf_win_height, "[PixelBox] : loading");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	// set custom GUI style
	GuiLoadStyleDark();

	// init asset system and VFS
	initAssetSystem();

	// init Screen system
	SetRootScreen(&ScrMainMenu);
	initDToolkit();

	// end
	prof_end();
}

void _FreeEngine() {
	// free screen system and current screen
	if (SCREEN && SCREEN->destroy) SCREEN->destroy();
	freeDToolkit();

	// refresh some settings
	refreshSettings();

	freeAssetSystem(); // and VFS
	CloseWindow();
	saveSattings();
	prof_unregister_thread();
	// DONE
}

static void _TickEngine() {
	prof_begin(PROF_GAMETICK);
	bool should_close = WindowShouldClose();

	// begin drawing
	prof_begin(PROF_FINDRAW);
	BeginDrawing();
	ClearBackground(RAYWHITE);
	prof_end(PROF_FINDRAW);

	// collect usused assets
	prof_begin(PROF_GC);
	collectAssets();
	prof_end(); 

	UpdateScreenSystem(); // here all shit goes :D

	// finalize drawing
	prof_begin(PROF_FINDRAW);
	EndDrawing();
	prof_end();

	prof_end();
	// save profiler state at new record
	prof_step();
}

// that's right!
void GlobalEngineEntryPoint() {
	_InitEngine();
	while (is_game_working) 
		_TickEngine();
	_FreeEngine();
}
