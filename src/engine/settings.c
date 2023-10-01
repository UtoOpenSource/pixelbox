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

#include "settings.h"

int conf_max_fps = 60;
bool conf_vsync = true;
int conf_win_width = 640;
int conf_win_height = 480;
bool conf_debug_mode = 0;

#include <stdbool.h>
#include <stdio.h>

static bool settread(void* p, size_t s, FILE* f) {
	bool res = false;
	if (!f) return false;
	return fread(p, s, 1, f) == 1;
}

static void settwrite(void* p, size_t s, FILE* f) {
	if (f) fwrite(p, s, 1, f);
}

#define READ(V, DEF) (settread(&V, sizeof(V), F) ? V : DEF)
#define LIMIT(V, A, B) ((V < A) ? A : ((V > B) ? B : V))
#define WRITE(V) settwrite(&V, sizeof(V), F)

#include <limits.h>

#include "version.h"

void reloadSettings() {
	FILE* F = fopen("config.bin", "rb");
	if (!F) perror("can't open config.bin!");
	conf_max_fps = READ(conf_max_fps, 60);
	conf_max_fps = LIMIT(conf_max_fps, 0, 240);
	conf_vsync = READ(conf_vsync, 1);
	conf_vsync = LIMIT((int)conf_vsync, 0, 1);
	conf_win_width = READ(conf_win_width, 640);
	conf_win_width = LIMIT(conf_win_width, 100, INT_MAX);
	conf_win_height = READ(conf_win_height, 480);
	conf_win_height = LIMIT(conf_win_height, 100, INT_MAX);
	conf_debug_mode = READ(conf_debug_mode, PIXELBOX_DEBUG);
	conf_debug_mode = LIMIT((int)conf_debug_mode, 0, 1);
	if (F) fclose(F);
}

void saveSattings() {
	FILE* F = fopen("config.bin", "wb");
	if (!F) perror("Can't save settings!");
	WRITE(conf_max_fps);
	WRITE(conf_vsync);
	WRITE(conf_win_width);
	WRITE(conf_win_height);
	WRITE(conf_debug_mode);
	if (F) fclose(F);
}

void refreshSettings() {
	conf_win_width = GetScreenWidth();
	conf_win_height = GetScreenHeight();
}
