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
#include "screen.h"

void drawProfiler(Rectangle rec);

static class Debug : public screen::Base {
	GuiWindowCtx win;

	void shown() {
		GuiWindowConf(&win, "debug", 
			(Rectangle){(float)GetScreenWidth() - 470, 0, 450, 350},
			true
		);
	}

	void hidden() {}

	bool drawgui() {
		GuiWindow(&win, [](Rectangle rec) {
			drawProfiler(rec);
		});	
		return false;
	}


	void update(float dt) {
		GuiWindowMove(&win);
		if (GuiWindowCollision(&win) || win.moving) {
			manager->refreshGui();
		};
	}

} _scr;

namespace screen {
	Base* Debug = &_scr;
};
