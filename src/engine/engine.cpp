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

#include "engine.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <exception>

extern "C" void GuiLoadStyleDark();

namespace engine {

static bool is_game_working = true;

void stop() { is_game_working = false; }

bool conf_vsync = true;
int  conf_max_fps = 70;
int  conf_win_width = 640;
int  conf_win_height = 480;

#include <limits.h>

static conf::Register _c1("conf_vsync", conf_vsync);
static conf::Register _c2("conf_max_fps", conf_max_fps, 1, 240);

static conf::HookedValue<conf::Integer> _hv1(
	nullptr, [](conf::Integer&){
		return GetScreenWidth();
	}, conf_win_width, 100, INT_MAX
);

static conf::HookedValue<conf::Integer> _hv2(
	nullptr, [](conf::Integer&){
		return GetScreenHeight();
	}, conf_win_height, 100, INT_MAX
);

static conf::Register _c3("conf_win_width", _hv1);
static conf::Register _c4("conf_win_height", _hv2);

lua_State* global = nullptr;

void init() {
	prof_register_thread();	 // init profiler
	prof_begin(PROF_INIT_FREE);
	conf::Reload(); // load settings

	// raylib - set log and vsync/fps limit
	SetTraceLogLevel(LOG_INFO);
	if (conf_vsync) {
		SetConfigFlags(FLAG_VSYNC_HINT);
	} else
		SetTargetFPS(conf_max_fps);

	// init raylib window
	InitWindow(conf_win_width, conf_win_height, "[PixelBox] : loading");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	// set custom GUI style
	GuiLoadStyleDark();

	// init asset system and VFS
	initAssetSystem();
	InitAudioDevice();

	global = lua::newState();
	luaL_dostring(global, "require 'lua/main'");

	// end
	prof_end();
}

void uninit() {
	// free screen system and current screen
	lua_close(global);

	CloseAudioDevice();

	// no need to refresh some settings now...
	conf::Save(); // save settings here, 'cause this is last chance

	freeAssetSystem();	// and VFS
	CloseWindow();
	conf::Destroy(); // we don't need all theese parameters anymore
	prof_unregister_thread();
	// DONE
}

static void tick() {
	prof_begin(PROF_GAMETICK);
	bool should_close = WindowShouldClose();

	// begin drawing
	prof_begin(PROF_FINDRAW);
	BeginDrawing();
	ClearBackground(RAYWHITE);
	prof_end();

	// collect usused assets
	prof_begin(PROF_GC);
	collectAssets();
	prof_end();

	// TODO: draw something
	perror("in loop");
	lua_State* L = global;
	lua_settop(L, 0);
	if (lua_getglobal(L, "engine_loop") != LUA_TFUNCTION ||
			lua_pcall(L, 0, 1, -1) != LUA_OK) {
		vflog("LUA: LOOP: %s", lua_tostring(L, -1));
		stop();
	};

	// finalize drawing
	prof_begin(PROF_FINDRAW);
	EndDrawing();
	prof_end();

	prof_end();
	// save profiler state at new record
	prof_step();
}


// that's right!
void join() {
	try {
		while (is_game_working) tick();
	} catch (const char* s) {
		fprintf(stderr, "uncaught exception : %s\n", s);
	} catch (std::exception& x) {
		fprintf(stderr, "uncaught std::exception %s\n", x.what());
	}
}

};
