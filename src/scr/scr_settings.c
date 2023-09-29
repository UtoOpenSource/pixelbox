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

#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void create() {

}

static void destroy() {

}

#include "settings.h"
#include <stdio.h>

static void draw() {
	Rectangle rec = GuiMenuWindow("Settings");
	Rectangle item = {rec.x, rec.y, 200, 20};

	int new_fps = GuiSliderBar(item, NULL, TextFormat("Max FPS : %i", conf_max_fps), conf_max_fps, 0, 240);
	if (new_fps != conf_max_fps) {
		conf_max_fps = new_fps;
		if (!conf_vsync) SetTargetFPS(conf_max_fps);
	}

	item.y += 25;

	if (GuiToggle(item, "Enable Vsync", conf_vsync) != conf_vsync) {
		conf_vsync = !conf_vsync;
		fprintf(stderr, "%i\n", (int)conf_vsync);
		if (conf_vsync) {
			SetTargetFPS(0);
			SetWindowState(FLAG_VSYNC_HINT);
		} else {
			ClearWindowState(FLAG_VSYNC_HINT);
			SetTargetFPS(conf_max_fps);
		}
	}

	item.y += 25;
	conf_debug_mode = GuiToggle(item, "Debug Mode", conf_debug_mode);
}

static void update() {

}

struct screen ScrSettings = {
	NULL, draw, update, create, destroy, NULL
};

