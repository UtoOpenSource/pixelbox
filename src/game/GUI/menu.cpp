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

namespace screen {
	extern Base* Server;
	extern Base* Client;
	extern Base* Local;
};

static class : public screen::Base {
	GuiWindowCtx win;

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
			Rectangle a = {rec.x, rec.y+5, rec.width, rec.height/4.0f-25};
			int h = a.height + 5;

			if (GuiButton(a, "Singleplayer")) {
				this->manager->setNext(screen::Local);
			};
			a.y += h;
			
			if (GuiButton(a, "Multiplayer")) {
				this->manager->setNext(screen::Client);
			};
			a.y += h;

			if (GuiButton(a, "Create Server")) {
				this->manager->setRoot(screen::Server);
			};
			a.y += h;

			if (GuiButton(a, "Settings")) {

			};
			a.y += h;
			
			
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
	Base* Menu = &_scr;
};
