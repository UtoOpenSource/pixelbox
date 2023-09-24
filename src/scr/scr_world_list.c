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
#include <stdio.h>
#include <time.h>

int GuiSavesViewEx(Rectangle bounds, struct save_file *saves, int count, int *focus, int *scrollIndex, int active);

static struct save_file* saves = NULL;
static size_t saves_len = 0;
static FilePathList files;
static int scroll, active;

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#endif

static void create() {
	WorldRefCreate();
	#if _WIN32
	_mkdir(".\\saves");
	#else
	mkdir("./saves", 0777);
	#endif

	files = LoadDirectoryFilesEx("./saves", ".db", false);
	if (!files.count || !files.paths) {
		saves = NULL; saves_len = 0;
		return;
	}

	saves = calloc(sizeof(struct save_file), files.count + 1);
	assert(saves);
	for (size_t i = 0; i < files.count; i++) {
		saves[i].path = files.paths[i];
		if (!getWorldInfo(files.paths[i], &saves[i].time, &saves[i].mode)) {
			saves[i].mode = -1;
			saves[i].time = 0;
		};
	}
	saves_len = files.count, scroll = 0, active = 0;
}

static void destroy() {
	free(saves);
	UnloadDirectoryFiles(files);
	WorldRefDestroy();
}


// world selection dialog
static void draw() {
	Rectangle rec = GuiMenuWindow("Select world file");
	rec.height -= 35*2;

	if (saves && saves_len) {
		active = GuiSavesViewEx(rec, saves, saves_len, NULL, &scroll, active);
	} else {
		active = -1;
		GuiLabel(rec, "No world files avaliable");
	}

	rec.y += rec.height + 5;
	rec.height = 25;
	rec.width  = rec.width / 2 - 2;

	int activez = active >= 0 ? active : 0;

	if (!saves || !saves_len || active < 0) GuiDisable();
	
	if (GuiButton(rec, "Delete")) {
		remove(saves[activez].path);
		saves_len -= 1;
		for (int i = activez; i < saves_len; i++) {
			saves[activez] = saves[activez + 1];
		}
	}

	rec.x += rec.width + 4;
	if (GuiButton(rec, "Open")) {
			SetWindowTitle(TextFormat("[pixelbox] : %s", saves[activez].path));
			openWorld(saves[activez].path);
			SetRootScreen(&ScrGamePlay);
	}
	GuiEnable();
	rec.y += 25;
	GuiLine(rec, TextFormat("%i : %s", active, saves? saves[activez].path : "nul"));
}

static void update() {

}

struct screen ScrWorldList = {
	NULL, draw, update, create, destroy
};

