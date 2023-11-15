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
 *
 * This file implements layouts for raygui, btw :)
 */

#pragma once
#include "raylib.h"
#include "raygui.h"

namespace rl {
	
	struct Layout {
		Rectangle bounds  = {0};
		Rectangle current = {0};
		bool horisontal = false; // vertical by default	
		int count = -1;
		float spacing = 5;
		public:
		void newline(int cnt = 0) {
			if (horisontal) {
				current.y += current.height + spacing;
				current.x = bounds.x;
				current.height = 0;
				current.width = 0;
			} else {
				current.x += current.width + spacing;
				current.y = bounds.y;
				current.width = 0;
				current.height = 0;
			}
			count = cnt;
		}
		Rectangle next(float w = -1, float h = -1) {
			if (count<=0 && ((horisontal && h <= 0) ||
					(!horisontal && w <= 0))) {
				throw "invalid layout calculation";
			}

			// set width value
			if (w < 0) {
				if (!horisontal) {
					// a,
					// b,
					// c
					w = bounds.width;
				} else if (count > 0) { 
					w = (bounds.x - current.x - spacing
						+ bounds.width - (current.width)) / (count--);
				} else throw "a";
			}

			if (h < 0) {
				if (horisontal) { // a, b, c
					h = bounds.height;
				} else if (count > 0) {
					h = ((bounds.y + bounds.height) - spacing
						- current.y - (current.height)) / (count--);
				} else throw "b";
			}

			Rectangle rec;
			if (horisontal) {
				current.x += current.width + spacing;
				current.width = w;
				if (current.x + current.width - spacing
						> bounds.width + bounds.x + spacing) {
					newline(count);
					current.height = h;
				}	
				rec = current;
				rec.height = h;
				if (current.height < h) current.height = h;
			} else {
				current.y += current.height + spacing;
				current.height = h;
				if (current.y + current.height - spacing
						> bounds.height + bounds.y + spacing) {
					newline(count);
					current.width = w;
				}	
				rec = current;
				rec.width = w;
				if (current.width < w) current.width = w;
			} 
			return rec;
		}
		void init(Rectangle pos, int cnt, bool horis) {
			bounds = pos;
			count = cnt;
			horisontal = horis;
			current.x = bounds.x;
			current.y = bounds.y;
		}
		float itemLen(int count) {
			if (horisontal)
				return (bounds.height / (float)count) - (spacing*count);
			else 
				return (bounds.width / (float)count) - (spacing*count);
		}
	};

};
