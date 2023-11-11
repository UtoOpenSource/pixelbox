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

static class : public screen::Windowed {
	void shown() {
		GuiWindowConf(&win, "loading", 
			(Rectangle){(float)GetScreenWidth() - 470, 0, 450, 350},
			true
		);
		GuiWindowCenter(&win);
	}

	void hidden() {}

	bool drawgui() {
		GuiWindow(&win, [this](Rectangle rec) {
			Rectangle a = {rec.x, rec.y+5, rec.width, rec.height/5.0f-25};
			int h = a.height + 5;

			if (GuiButton(a, "Open World")) {
				this->manager->setNext(screen::WorldList);
			};
			a.y += h;
			
			if (GuiButton(a, "Connect Server")) {
				this->manager->setNext(screen::ServerList);
			};
			a.y += h;

			if (GuiButton(a, "Settings")) {
				this->manager->setNext(screen::Settings);
			};
			a.y += h;

			if (GuiButton(a, "Exit Program")) {
				engine::stop();
			};
			a.y += h;
			
		});	
		return false;
	}
} _scr;

namespace screen {
	Base* Menu = &_scr;
};
