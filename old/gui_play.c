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

#include <raylib.h>
#include <stdio.h>

#include "game.h"
#include "pixel.h"
#include "raygui.h"

Rectangle winrec = (Rectangle){0, 100, 400, 110};

static const char *tabs[] = {"world", "pallete", "about", NULL};

static int active_tab = 1;
static int window_hidden = 0;
int GuiTabBarEx(Rectangle bounds, int width, int closeable,
								const char **text, int count, int *active);
void GuiLoadStyleDark(void);	// CALL ONLY ONCE!
int allowupdate = 0;

void drawGUI() {
	GuiPanel(winrec, TextFormat("[PIXELBOX v.%1.1f] (%i FPS)", 1.0,
															(int)GetFPS()));
	Rectangle rec =
			(Rectangle){// Hide button rectangle
									winrec.x + winrec.width - 20, winrec.y + 2, 20, 20};

	if (GuiButton(rec, GuiIconText(window_hidden ? ICON_ARROW_UP
																							 : ICON_ARROW_DOWN,
																 ""))) {
		window_hidden = !window_hidden;
	}

	if (window_hidden) return;	// :)

	// content rectangle
	rec = (Rectangle){winrec.x, winrec.y + 25, winrec.width,
										winrec.height - 25};

	int old = rec.height;
	rec.height = 20;
	GuiTabBarEx(rec, 80, false, tabs, 3, &active_tab);
	rec.y += 25;
	rec.height = old - 25;
	rec.x += 5;
	rec.width -= 5;

	Rectangle item = (Rectangle){rec.x, rec.y, rec.width / 2, 25};

	switch (active_tab) {
		case 0: {	 // world
			allowupdate = GuiToggle(item, "Enable Physic", allowupdate);
		}; break;
		case 1: {	 // pallete

		}; break;
		case 2: {	 // about
			GuiLine(rec, "about");
		}; break;
		default:

			break;
	}
}

int UpdateGUI() {	 // AND player interactions are here
	winrec =
			(Rectangle){0, GetScreenHeight() - (window_hidden ? 25 : 200),
									400, (window_hidden ? 25 : 205)};
	GuiLock();
	if (CheckCollisionPointRec(GetMousePosition(), winrec)) {
		// gui
		GuiUnlock();
	} else {	// interact
		if (IsMouseButtonDown(0)) {
			Vector2 md = GetMouseDelta();
			cam.target.x -= md.x / cam.zoom;
			cam.target.y -= md.y / cam.zoom;
		}

		cam.zoom += GetMouseWheelMove() * 0.15 * cam.zoom;
		if (cam.zoom < 0.01) cam.zoom = 0.01;
		if (cam.zoom > 100) cam.zoom = 100;

		// input
		if (IsMouseButtonDown(1)) {
			Vector2 dpos = GetMouseDelta();
			Vector2 cpos = GetMousePosition();
			dpos.x = cpos.x - dpos.x;
			dpos.y = cpos.y - dpos.y;
			dpos = GetScreenToWorld2D(dpos, cam);
			cpos = GetScreenToWorld2D(cpos, cam);
			setline(cpos.x, cpos.y, dpos.x, dpos.y);
		}

		if (IsKeyDown(KEY_U)) {
			updateWorld();
		}
	}
	drawGUI();
	if (allowupdate) updateWorld();
}
