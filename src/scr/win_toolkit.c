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
#include "profiler.h"

static int color_gradient = -1;
static int color_material = -1;
static float rainbow_v = 0, rainbow_speed = 5;

void initToolkit() {

}

void freeToolkit() {
	GuiUnlock();
}

static uint8_t b_rainbow() {
	return rainbow_v;
}

static uint8_t b_random() {
	return randomNumber();
}

static uint8_t b_noise() {
	return noise1(rainbow_v/128.0) * 255;
}

static uint8_t getbval(int v) {
	if (v < 0) switch(v) {
		case -3 : return b_noise();
		case -2 : return b_rainbow();
		case -1 : return b_random();
		default : break;
	}
	return v;
}

static uint8_t getcolor(int8_t m, int8_t g) {
	uint8_t color = 0;
	color = getbval(m);
	color = color << 2;
	color = color | getbval(g) & 3;
	return color;
}

void setline (int x0, int y0, int x1, int y1) {
	int dx = ABS(x1 - x0);
	int dy = ABS(y1 - y0) * -1;
	int stepx = x0 < x1 ? 1 : -1;
	int stepy = y0 < y1 ? 1 : -1;
	
	int err = dx + dy, e2 = 0;
	int limit = 0;
	
	while (limit < 255) {
		limit++;
		setWorldPixel(x0, y0, 
			getcolor(color_material, color_gradient), MODE_READ);
		markWorldUpdate(x0, y0);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) {
			err = err + dy;
			x0  = x0 + stepx;
		}
		if (e2 <= dx) {
			err = err + dx;
			y0  = y0 + stepy;
		}
	}
}

Rectangle winrec = (Rectangle){
	0, 100,
	400, 110
};

static const char* tabs[] = {
	"world",
	"pallete",
	"stats",
	"profiler",
	NULL
};

static int active_tab = 1;
static int allowupdate = 0;
static int window_hidden = 0;

bool GuiColorButton(Rectangle bounds, Color color, const char *text);

static Rectangle getbrec(int i, Rectangle rec) {
	return (Rectangle){
		rec.x + 2 + (i % (350/27))*25,
		rec.y + 2 + (i/(350/27))*25,
		25, 25
	};	
}

static const char* getbtext(int type) {
	if (type < 0) {
		switch(type) {
			case -1: return "RNG";
			case -2: return "RAI";
			case -3: return "PER";
			default: return "?";
		}
	} else return TextFormat("%i", (int)type);
}

static int8_t bnorm(int v) {
	if (v < 0) return 0;
	return v;
}

static void colorbutton(Rectangle rec, int type) {
	int v = getcolor(type, bnorm(color_gradient));
	if (GuiColorButton(rec, 
				getPixelColor(v), 
				getbtext(type)
			)) {
		color_material = type;
	};
	if (type == color_material)
		DrawRectangleLinesEx(rec, 2, YELLOW);
}

static Rectangle getgrec(int i, Rectangle rec) {
	return (Rectangle) {
		rec.x + 12 + (12+2)*25,
		rec.y + 2 + (i)*25,
		25, 25
	};	
}



// color transformation
static uint8_t HItoC(int v) {
	uint8_t a = v & 3;
	uint8_t b = (v >> 2) & 3;
	uint8_t c = (v >> 4) & 3;

	uint8_t r = (a==1) + (b==3) + (c==2);
	uint8_t g = (a==2) + (b==1) + (c==3);
	uint8_t bl = (a==3) + (b==2) + (c==1);

	return r | (g << 2) | (bl << 4);
}

static void gradbutton(Rectangle rec, int type) {
	int v = getcolor(bnorm(color_material), type);
	if (GuiColorButton(rec, 
				getPixelColor(v), 
				getbtext(type)
			)) {
		color_gradient = type;
	};
	if (type == color_gradient)
		DrawRectangleLinesEx(rec, 2, GREEN);
}

static void drawPallete(Rectangle rec) {
	Rectangle item = (Rectangle){
		rec.x, rec.y,
		rec.width/3, 25
	};

	for (int i = 0; i < 64; i++) {
		item = getbrec(i, rec); 
		colorbutton(item, i);	
	}

	for (int i = 1; i < 4; i++) {
		item = getbrec(68+i, rec); 
		colorbutton(item, 0-i);
	}

	for (int i = 0; i < 4; i++) {
		item = getgrec(i, rec);
		gradbutton(item, i);
	}

	item = getgrec(5, rec);
	gradbutton(item, -1);

}


void drawProfiler(Rectangle rec);
void debugRender(Rectangle rec);
void debugAllocator(Rectangle rec);
static int  active_stat = 0;
static int  active_hash = 0;

void debugHash(Rectangle rec) {
		
	Rectangle item = {rec.x, rec.y-20, 50, 20};
	active_hash = GuiToggleGroup(item, "Map;Load;Save;Update", active_hash);

	struct chunkmap* m = NULL;
	if (active_hash == 0) m = &World.map;
	else if (active_hash == 1) m = &World.load;
	else if (active_hash == 2) m = &World.save;
	else m = &World.update;

	for (int i = 0; i < MAPLEN; i++) {
		struct chunk *o = m->data[i];
		int j = 0;
		DrawPixel(rec.x + i, rec.y - 1, YELLOW);
		if (m->g) {
			assert(active_hash == 0);
			while (o) {
				DrawPixel(rec.x + i, rec.y + j, MAGENTA);
				o = o->next;
				j++;
			}
		} else {
			while (o) {
				DrawPixel(rec.x + i, rec.y + j, PINK);
				o = o->next2;
				j++;
			}
		}
	}
}

static void drawStats(Rectangle rec) {
	Rectangle item = (Rectangle){
		rec.x, rec.y,
		rec.width-25, 10
	};

	GuiLabel(item, TextFormat("CAM POS = %i:%i[in chunks=%i:%i]", 
		(int)cam.target.x,
		(int)cam.target.y,
		(unsigned int)cam.target.x/CHUNK_WIDTH,
		(unsigned int)cam.target.y/CHUNK_WIDTH
		)
	);
	item.y += item.height + 5;
	GuiLabel(item, TextFormat("In Game time : %i:%i:%i:%i", 
		World.playtime/60/60/24, 
		(World.playtime/60/60)%24,
		(World.playtime/60)%60,
		World.playtime%60)
	);
	int oldx = item.x;
	int oldw = item.width;
	item.x += 200;
	item.width -= 300;
	item.height += 10;
	active_stat = GuiToggleGroup(item, "Hash;Render;Allocator", active_stat);
	item.x = oldx;
	item.width = oldw; 
	item.y += item.height + 5;

	rec.height = rec.y + rec.height - item.y;
	rec.y = item.y;
	if (active_stat == 2) debugAllocator(rec);
	else if (active_stat == 1) debugRender(rec);
	else debugHash(rec);
}

extern struct screen ScrSaveProc;

void drawToolkit() {
	GuiPanel(winrec, TextFormat("[PIXELBOX] (%i FPS)", (int)GetFPS()));
	Rectangle rec = (Rectangle) { // Hide button rectangle
		winrec.x + winrec.width - 20, winrec.y + 2,
		20, 20
	};

	Rectangle recc = (Rectangle) { // color rectangle
		winrec.x + winrec.width - 40, winrec.y + 2,
		20, 20
	};

	int icon_kind = window_hidden ? ICON_ARROW_UP : ICON_ARROW_DOWN;

	if (GuiButton(rec, GuiIconText(icon_kind, ""))) {
		window_hidden = !window_hidden;
	}

	GuiColorButton(recc, 
		getPixelColor(getcolor(color_material, color_gradient)),
		""
	);

	if (window_hidden) return; // :)
	
	// content rectangle
	rec = (Rectangle) {
		winrec.x, winrec.y + 25,
		winrec.width, winrec.height - 25
	};

	int old = rec.height;
	rec.height = 20;
	GuiTabBarEx(rec, 80, false, tabs, 4, &active_tab);
	rec.y += 25;
	rec.height = old - 25;
	rec.x += 5;
	rec.width -= 5;

	Rectangle item = (Rectangle){
		rec.x, rec.y,
		rec.width/2, 25
	};

	switch(active_tab) {
		case 0 : {// world
			if (GuiToggle(item, "Enable Physic", allowupdate) != allowupdate) {
				World.is_update_enabled = !allowupdate;
				allowupdate = !allowupdate;
			}
			item.y += item.height + 5;

			GuiButton(item, "Dummy");
			item.y += item.height + 5;

			GuiButton(item, "Dummy");
			item.y += item.height + 5;

			GuiButton(item, "Dummy");
			item.y += item.height + 5;

			if (GuiButton(item, "Save and exit")) {
				SetRootScreen(&ScrSaveProc);	
			};
		}; break;
		case 1 : {// pallete 
			drawPallete(rec);
		}; break;
		case 2 : {// stats
			drawStats(rec);
		}; break;
		case 3 :
			drawProfiler(rec);
		break;
		default :
			
		break;
	}
}

static double old_time = 0.0;

bool updateToolkit() { 
	float dt = GetTime() - old_time;
	old_time = GetTime();
	rainbow_v += rainbow_speed * dt;
	if (rainbow_v > 1000) rainbow_v = -1000;

	winrec = (Rectangle){
		0, GetScreenHeight() - (window_hidden ? 25 : 200),
		400, (window_hidden ? 25 : 205)
	};
	GuiLock();
	if (CheckCollisionPointRec(GetMousePosition(), winrec)) {
		// gui
		GuiUnlock();
		return true;
	}
	return false;
}
