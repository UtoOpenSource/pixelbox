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

static Rectangle dwinrec = (Rectangle){0, 100, 400, 110};

void initDToolkit() {
	dwinrec = (Rectangle){GetScreenWidth() - 470, 0, 450, 20};
}

void freeDToolkit() {}

static void controlTab(Rectangle rec) {
	Rectangle item = (Rectangle){rec.x, rec.y, rec.width - 25, 10};
}

#include "version.h"

bool CheckCurrentScreen(struct screen* CURR);
bool safeWorld() { return CheckCurrentScreen(&ScrGamePlay); }

void drawDToolkit() {
	GuiLock();
	if (CheckCollisionPointRec(GetMousePosition(), dwinrec)) {
		GuiUnlock();
	}

	GuiPanel(dwinrec, "DEBUG");
	Rectangle rec = (Rectangle){
			// Hide button rectangle
			dwinrec.x + dwinrec.width - 20, dwinrec.y + 2, 20, 20};

	int icon_kind = window_hidden ? ICON_ARROW_UP : ICON_ARROW_DOWN;

	if (GuiButton(rec, GuiIconText(icon_kind, ""))) {
		window_hidden = !window_hidden;
	}

	if (window_hidden) {
		GuiUnlock();
		return;	 // :)
	}

	// content rectangle
	rec = (Rectangle){dwinrec.x, dwinrec.y + 25, dwinrec.width,
										dwinrec.height - 25};

	int old = rec.height;
	rec.height = 20;
	GuiUnlock();
}

static int move = 0;

bool updateDToolkit() {
	/*dwinrec = (Rectangle){
		GetScreenWidth() - 500, 0,
		450, (window_hidden ? 20 : 250)
	}; */

	Rectangle winbarrec = (Rectangle){dwinrec.x, dwinrec.y, 400, 20};

	dwinrec.height = (window_hidden ? 20 : 250);

	if (move && IsMouseButtonDown(0)) {
		Vector2 delta = GetMouseDelta();
		dwinrec.x += delta.x;
		dwinrec.y += delta.y;
	} else
		move = 0;

	if (dwinrec.x < 0) dwinrec.x = 0;
	if (dwinrec.y < 0) dwinrec.y = 0;
	if (dwinrec.x + dwinrec.width > GetScreenWidth())
		dwinrec.x = GetScreenWidth() - dwinrec.width;
	if (dwinrec.y + dwinrec.height > GetScreenHeight())
		dwinrec.y = GetScreenHeight() - dwinrec.height;

	if (CheckCollisionPointRec(GetMousePosition(), dwinrec)) {
		if (CheckCollisionPointRec(GetMousePosition(), winbarrec) &&
				IsMouseButtonPressed(0)) {
			move = 1;
		}
		return true;
	}
	return false;
}
