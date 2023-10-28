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
#include "infrastructure.hpp"

static RNG _randgen;

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
		GuiWindow(&win, [](Rectangle rec) {
			Rectangle a = {rec.x, rec.y, rec.width, rec.height/2.0f};
			if (GuiButton(a, "Problems?")) {
				assets::PlaySound(assets::LookupID("assets/levelup.ogg"), 
					1, 0.9 + _randgen.getn() * 0.2
				);
			};
			a.y += a.height;
			assets::GuiTexture(a, assets::LookupID("assets/shit_maker.png"));
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
	Base* Local = &_scr;
};
