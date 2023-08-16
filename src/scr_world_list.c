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
#include <time.h>

static const char** list = NULL;
static FilePathList files;
static int list_len, scroll, active;

static void create() {
	WorldRefCreate();
	files = LoadDirectoryFilesEx("./saves", ".db", false);
	if (!files.count || !files.paths) {
		list = NULL; list_len = 0;
		return;
	}
	list = (const char**)calloc(sizeof(char*), files.count + 1);
	assert(list);
	memcpy(list, files.paths, files.count*sizeof(char*));
	list_len = files.count, scroll = 0, active = 0;
}

static void destroy() {
	free(list);
	UnloadDirectoryFiles(files);
	WorldRefDestroy();
}

// world selection dialog
static void draw() {
	GuiEnable();

	Rectangle rec = GuiMenuWindow("Select world file");
	rec.height -= 35*2;

	if (list && list_len) {
		active = GuiListViewEx(rec, list, list_len, NULL, &scroll, active);
	} else {
		active = -1;
		GuiLine(rec, "No world files avaliable");
	}

	rec.y += rec.height + 5;
	rec.height = 25;
	rec.width  = rec.width / 2 - 2;

	int activez = active >= 0 ? active : 0;

	if (!list || !list_len || active < 0) GuiDisable();
	
	if (GuiButton(rec, "Delete")) {
		remove(list[activez]);
		list_len -= 1;
		for (int i = activez; i < list_len; i++) {
			list[activez] = list[activez + 1];
		}
	}

	rec.x += rec.width + 4;
	if (GuiButton(rec, "Open")) {
			openWorld(list[activez]);
			SetRootScreen(&ScrGamePlay);
			/*uint64_t len = strlen(list[active]);
			if (world_db_path) free(world_db_path);
			world_db_path = malloc(len+1);
			assert(world_db_path);
			memcpy(world_db_path, list[active], len+1);
			*/
	}
	GuiEnable();
	rec.y += 25;
	GuiLine(rec, TextFormat("%i : %s", active, list? list[activez] : "nul"));
}

static void update() {

}

struct screen ScrWorldList = {
	NULL, draw, update, create, destroy
};

