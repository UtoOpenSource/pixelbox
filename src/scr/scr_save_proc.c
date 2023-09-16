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
#include "libs/c89threads.h"

static int stage = 0;
static c89thrd_t thread = {0};
static c89mtx_t  done = {0};

static void create() {
	WorldRefCreate();
	c89mtx_init(&done, 0);
	stage = 0;
}

static void destroy() {
	c89mtx_destroy(&done);
	assert(c89thrd_join(thread, NULL) == 0);
	WorldRefDestroy();
}

static int func(void* unused) {
	(void*) unused;
	c89mtx_lock(&done);
	collectAnything();
	flushWorld();	
	c89mtx_unlock(&done);
	return 0;
}

static void draw() {
	if (!stage) {
		if (c89thrd_create(&thread, func, NULL) != 0) {
			assert(0 && "thread creation failed!");
		};
	}

	if (stage) {
		if (c89mtx_trylock(&done)==0) {
			c89mtx_unlock(&done);
			SetRootScreen(&ScrMainMenu);
		}
	}

	DrawRectangleRec(
		(Rectangle) {
			0, 0,
			GetScreenWidth(),
			GetScreenHeight()
		},
		BLACK
	);

	GuiPanel(
		(Rectangle) {
			GetScreenWidth()/3,
			GetScreenHeight()/3,
			GetScreenWidth()/3,
			GetScreenHeight()/3
		}, "Saving world..."
	);

	// draw
	stage++;
}

static void update() {

}

static void onclose() {

}

struct screen ScrSaveProc = {
	NULL, draw, update, create, destroy, onclose
};

