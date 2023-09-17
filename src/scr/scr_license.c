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

static const char* license = 
"Pixelbox - Infinite 2D sandbox game\n"
"Copyright (C) 2023 UtoECat\n"
"\n"
"This program is free software: you can redistribute it and/or modify "
"it under the terms of the GNU General Public License as published by "
"the Free Software Foundation, either version 3 of the License, or "
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful, "
"but WITHOUT ANY WARRANTY; without even the implied warranty of "
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License "
"along with this program.  If not, see <https://www.gnu.org/licenses/>\n"
;

static void create() {

}

static void destroy() {

}

static Vector2 scroll = {0, 0};

static void draw() {
	Rectangle rec = GuiMenuWindow("License");
	rec.height -= 10;
	Rectangle con = {0, 0, 400, 350};
	Rectangle out = GuiScrollPanel(rec, NULL, con, &scroll);
	BeginScissorMode(out.x, out.y, out.width, out.height);
	con.y = out.y + scroll.y;
	con.x = out.x + scroll.x;
	GuiTextView(con, license);
	con.y += con.height - 40;
	con.height = 20;
	con.width  = con.width / 2 - 5;
	if (GuiButton(con, "Get Source Code")) {
		OpenURL("https://github.com/UtoECat/pixelbox");
	}
	EndScissorMode();
}

static void update() {

}

struct screen ScrLicense = {
	NULL, draw, update, create, destroy
};

