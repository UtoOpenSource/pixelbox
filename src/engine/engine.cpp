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
#include <string>
#include <vector>

extern class ClockSource {
	double oldtime; 
	float  frame_time;
	void init();

	public:
	double time();
	float  delta();
	float  tick();
	ClockSource();
	~ClockSource(); 

} __clocksource;

extern "C" void GuiLoadStyleDark();

std::string getConfigDir(const char* appname);

namespace engine {

// engine has it's own config!
::conf::Manager settings(getConfigDir("pixelbox").c_str(), "engine.ini");

bool verbose = false;
static bool is_game_working = true;

void stop() { is_game_working = false; }

bool conf_vsync = true;
int  conf_max_fps = 70;
int  conf_win_width = 640;
int  conf_win_height = 480;

#include <limits.h>

static conf::Register _c1(settings, "conf_vsync", conf_vsync);
static conf::Register _c2(settings, "conf_max_fps", conf_max_fps, 1, 240);

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

static conf::Register _c3(settings, "conf_win_width", _hv1);
static conf::Register _c4(settings, "conf_win_height", _hv2);


/* uses _clocksource */
double getTime() {
	return __clocksource.time();
}

float  clockTick() {
	return __clocksource.tick();
}

float  deltaTime() {
	return __clocksource.delta();
}

static int init_stat = 0;

void init(bool gui, const char* const* names) {

	if (init_stat) throw "double init";
	init_stat = 1 + int(gui);

	prof::register_thread(names);	 // init profiler
	prof::begin(1);
	settings.reload(); // load settings

	// raylib - set log and vsync/fps limit
	SetTraceLogLevel(verbose ? LOG_INFO : LOG_ERROR);

	if (gui) { // don't init raylib and audio in CLI state
		if (conf_vsync) {
			SetConfigFlags(FLAG_VSYNC_HINT);
		} else
			SetTargetFPS(conf_max_fps);

		// init raylib window
		InitWindow(conf_win_width, conf_win_height, "[PixelBox] : loading");
		SetWindowState(FLAG_WINDOW_RESIZABLE);

		// set custom GUI style
		GuiLoadStyleDark();

		// init asset system and VFS (asset system can't work without raylib)
		assets::init();

		// audio without window will be scary :D
		InitAudioDevice();
	}
	
	// end
	prof::end();
}

static std::vector<UninitHook*> hooks;

void   subscribeUninit(UninitHook* hook) {
	if (!hook) throw "oh no";
	hooks.push_back(hook);
}

void uninit() {
	if (!init_stat) throw "oh no";

	for (auto &i : hooks) {
		i->destroy();
	}

	if (init_stat > 1) { // if GUI
		// free audio
		CloseAudioDevice();

		settings.save(); // save settings here, 'cause this is last chance

		assets::free();	// and VFS
		CloseWindow();
	}

	prof::unregister_thread();
	// DONE
}

static int (*callback) (double dt, bool should_close); 

static void gui_tick() {
	prof::begin(DEF_ENT_TICK);
	bool should_close = WindowShouldClose();

	// begin drawing
	BeginDrawing();
	ClearBackground(RAYWHITE);

	// collect usused assets
	prof::begin(DEF_ENT_GC);
	assets::collect();
	prof::end();

	if (callback(clockTick(), should_close) != 0)
		stop();

	// finalize drawing
	EndDrawing();

	prof::end();
	// save profiler state at new record
	prof::step();
}

// only GAMEPLAY YAY
static void cli_tick() {
	prof::begin(DEF_ENT_TICK);

	if (callback(clockTick(), 0) != 0) // can't be closed from outside btw :D
		stop();	

	prof::end();
	// save profiler state at new record
	prof::step();
}


// that's right!
void join(int (*cb)(double, bool)) {
	callback = cb;
	if (!cb) throw "callback function is required to join engine loop!";

	try {
		if (init_stat > 1) { // GUI
			while (is_game_working) gui_tick();
		} else {
			while (is_game_working) cli_tick();
		}
	} catch (const char* s) {
		fprintf(stderr, "uncaught exception : %s\n", s);
	} catch (char const* s) {
		fprintf(stderr, "uncaught exception : %s\n", s);
	}catch (std::exception& x) {
		fprintf(stderr, "uncaught std::exception %s\n", x.what());
	}
}

// utility


int vaddformat(std::string& buff, const char* fmt, va_list orig) {
	va_list args;

	// get length
	va_copy(args, orig);
	int size = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	if (size <= 0) return size;

	size_t curr = buff.size();
	buff.resize(curr + size);
	vsnprintf(&buff[0] + curr, size + 1, fmt, orig);
	return size;
}

int addformat(std::string& buff, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int res = vaddformat(buff, fmt, args);
	va_end(args);
	return res;
}

};

#include <new>

void* operator new(size_t sz) {
	void* p = malloc(sz);
	if (!p) throw "OOM";
	return p;
}

void* operator new[](size_t sz) {
	void* p = malloc(sz);
	if (!p) throw "OOM";
	return p;
}

void* operator new[](size_t sz, size_t mul) {
	void* p = malloc(sz * mul);
	if (!p) throw "OOM";
	return p;
}

void operator delete(void* p) {
	free(p);
}

void operator delete[](void* p) {
	free(p);
}

void operator delete(void* p, size_t) {
	free(p);
}

void operator delete[](void* p, size_t) {
	free(p);
}
