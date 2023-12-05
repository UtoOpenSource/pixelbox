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
#include "game.hpp"

#include "scrtotal.hpp"

static RNG _randgen;

// see raygui.c
extern "C" int GuiTabBarEx(Rectangle bounds, int width, int closeable, const char **text, int count, int *active);

static class : public screen::Windowed {

	void shown() {
		GuiWindowConf(&win, "Settings", 
			(Rectangle){(float)GetScreenWidth() - 470, 0, 450, 350},
			true
		);
		GuiWindowCenter(&win);
	}

	void hidden() {}

	const char* tab_names[2] = {
		"engine", "game"
	};
	int active_tab = 0;
	Vector2 scroll = {0};
	float predicted_height = 0;

	bool drawgui() {
		GuiWindow(&win, [this](Rectangle rec) {
			Rectangle rtab = {rec.x, rec.y, rec.width, 25};
			GuiTabBarEx(rtab, 90, false, tab_names, 2, &active_tab);

			Rectangle a = {rec.x, rec.y+25, rec.width, rec.height-25};

			int count = 10;
			float ih = 20;
			float is = 5;

			predicted_height = count * (ih + is);

			Rectangle con = {0, 0, a.width/2.0f, predicted_height};
			a = GuiScrollPanel(a, NULL, con, &scroll);  

			::conf::Manager* man = nullptr;
			switch(active_tab) {
				case 0: man = &engine::settings; break;
				default: break;
			};

			if (!man) return;
			BeginScissorMode(a.x, a.y, a.width, a.height);
			auto* p = man->getList();

			float x = a.x;
			float y = a.y + scroll.y;

			while (p) {
				p->value->showGUI(p->id, x, y, a.width, ih);
				y += ih + is;
				p = p->next;
			}

			EndScissorMode();
		});	
		return false;
	}

} _scr;

namespace screen {
	Base* Settings = &_scr;
};
