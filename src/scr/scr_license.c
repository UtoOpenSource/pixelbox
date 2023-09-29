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

#include "engine.h"
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

static const char* tabs[] = {
	"about",
	"license",
	"resources",
	"GNU GPL",
	NULL
};

static int active_tab = 0;

#include "assets.h"

float getStringHeight(const char* str);

static float getHeight() {
	switch(active_tab) {
		case 0: return 350;
		case 1: return 350;
		case 2: return 50;
		case 3: {
			AssetID a = LookupAssetID("assets/license.txt");
			return getStringHeight(GetStringAsset(a));
		};
	}
	return 350;
}

static void draw() {
	Rectangle rec = GuiMenuWindow("License");
	rec.height -= 30;
	Rectangle bar = {rec.x, rec.y, rec.width, 20};

	GuiTabBarEx(bar, 100, 0, tabs, 4, &active_tab);

	rec.y += 20;
	Rectangle con = {0, 0, rec.width > 400 ? rec.width : 400, getHeight()};

	Rectangle out = GuiScrollPanel(rec, NULL, con, &scroll);
	BeginScissorMode(out.x, out.y, out.width, out.height);
	con.y = out.y + scroll.y;
	con.x = out.x + scroll.x;
	switch (active_tab) {
		case 0: {
			Rectangle item = {con.x, con.y, 200, 200};
			item.x += con.width/2 - 100;
			GuiAssetTexture(item, LookupAssetID("assets/shit_maker.png"));
			item.y += item.height + 5;
			item.x = con.x;
			item.height = 20;
			item.width = con.width;
			int old = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
			GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
			GuiLabel(item, "By UtoECat");
			GuiSetStyle(LABEL, TEXT_ALIGNMENT, old);
			item.y += item.height;
		};
		break;
		case 1:
		GuiTextView(con, license);
		break;
		case 2:
			con.y += con.height - 40;
			con.height = 20;
			con.width  = con.width / 2 - 5;
			if (GuiButton(con, "Get Source Code")) {
				OpenURL("https://github.com/UtoECat/pixelbox");
			}
		break;
		case 3:
			AssetID a = LookupAssetID("assets/license.txt");
			GuiTextView(con, GetStringAsset(a));
		break;
	}
	EndScissorMode();
}

static void update() {

}

struct screen ScrLicense = {
	NULL, draw, update, create, destroy, NULL
};

