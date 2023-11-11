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

#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "raygui.h"

namespace conf {

	void   Flag::showGUI(const char* name, float x, float y, float w, float h) {
		Rectangle rec = {x, y, w, h};	
		*value = GuiToggle(rec, TextFormat("%s : %s", name, *value?"on":"off"), *value);
	}

	void   Unsigned::showGUI(const char* name, float x, float y, float w, float h) {
		Rectangle rec = {x, y, w/2.0f, h};	
		float vv = *value;
		vv = GuiSliderBar(rec, NULL, name, vv, min, max);
		*value = vv;
	}

	void   Integer::showGUI(const char* name, float x, float y, float w, float h) {
		Rectangle rec = {x, y, w, h};	
		float vv = *value;
		vv = GuiSliderBar(rec, NULL, NULL, vv, min, max);
		
		int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
		GuiLabel(rec, TextFormat("%s : %1.0f", name, vv));
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);

		*value = vv;
	}

	void Float::showGUI(const char* name, float x, float y, float w, float h) {
		Rectangle rec = {x, y, w, h};	
		*value = GuiSliderBar(rec, NULL, NULL, *value, min, max);
		
		int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
		GuiLabel(rec, TextFormat("%s : %1.0f", name, *value));
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);
	}

};


