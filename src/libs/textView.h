/*
** This file is a part of PixelBox - infinite sandbox game
** Copyright (C) 2021-2023 UtoECat
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <math.h>

/*******************************************************
 *
 * TERMINAL
 *
 *******************************************************/

#include <ctype.h>

static const char* skipspaces(const char* text) {
	while (*text == ' ' || *text == '\t') {
		text++;
	}
	return text;
}

static const char* skipalnum(const char* cstr) {
	const char* text = skipspaces(cstr);
	while (!isspace(*text) && *text != '\n' && *text != '\0') {
		text++;
	}
	return text;
}


float getCodepointWidth(Font font, int c) {
	int idx = GetGlyphIndex(font, c);
	// CONST?
	float fontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
	float scaleFactor = fontSize/font.baseSize;
	float fontSpaceing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);

	float w = (font.glyphs[idx].advanceX ? font.glyphs[idx].advanceX : font.recs[idx].width);
	return w * scaleFactor + fontSpaceing;
}

float getSubStrWidth(Font font, const char* c, const char* end, float maxwidth) {
	float width = 0.0;
	int lc = 0, lsz = 0;

	for (; c < end; c += lsz) {
		lc = GetCodepointNext(c, &lsz);
		if (lc == 0x3f) lsz = 1; // bad codepoint

		float w = getCodepointWidth(font, lc);
		if (width + w > maxwidth) return width; // ok
		width += w;
	}
	return width;
}

static int drawTextWord(Rectangle rec, Vector2* pos, const char** ptr, Color tint) {
	float fontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
	Font font = GuiGetFont();

	// get word size
	const char* word = skipspaces(*ptr); // word's first char
	const char* dst  = skipalnum(word); // first char after the word
	const char* c    = *ptr;

	// 'cause we can return somewhere below
	*ptr = (word > c) ? word-1 : c;

	// 'draw' spaces
	{
		float sw = getCodepointWidth(font, ' ');
		for (; c < word; c++) {
			if (pos->x + sw > rec.x + rec.width) { // too many spaces :/
				pos->x = rec.width; // hint
				return -1; // CALLER must do all dirty stuff with newline, not we're
			}
			pos->x += sw;
		}
	}

	float wwidth = getSubStrWidth(font, c, dst, rec.width); // because
	if (pos->x + wwidth > rec.x + rec.width) {
		return -1; // CALLER must do all dirty stuff with newline!
	}

	// c must = word here

	// draw word
	{
		int lc = 0, lsz = 0; // codepoint and it's size 
		for (; c < dst; c += lsz) {
			lc = GetCodepointNext(c, &lsz);
			if (lc == 0x3f) lsz = 1; // bad codepoint
	
			float w = getCodepointWidth(font, lc);
			if (pos->x > rec.x + rec.width) { // return the rest
				*ptr = c;
				return -1; // CALLER AGAIN
			}
			// DRAW IT IN OTHER CASE
			if (pos->y >= rec.y && pos->y < rec.y + rec.height)
				DrawTextCodepoint(font, lc, *pos, fontSize, tint);
			pos->x += w; // ok
		}
	}

	*ptr = c; // holy yeah
	return 1;
}

float getLineHeight() {
	Font font = GuiGetFont();
	float fontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
	float scaleFactor = fontSize/font.baseSize;
	return font.baseSize * scaleFactor + 3;
}

float getStringHeight(const char* str) {
	float lh = getLineHeight();
	float h = 0;

	for (; *str != '\0'; str++) {
		if (*str == '\n') {
			h += lh;
		}
	}
	return h;
}

int GuiTextView(Rectangle rec, const char* src) {
	const char* c = src;
	Vector2 pos = {rec.x, rec.y};
	Font font = GuiGetFont();

	// setup varables
	float fontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
	float fontSpaceing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);
	float scaleFactor = fontSize/font.baseSize;
	Color tint = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));

	while (*c != '\0' && pos.y < rec.y+rec.height) {
		if (drawTextWord(rec, &pos, &c, tint) < 0 || *c == '\n') {
			pos.y += (float)font.baseSize * scaleFactor + 3; // newline
			pos.x  = rec.x; 
		};
		if (*c == '\n') c++;
	}

	//GuiSetStyle(DEFAULT, TEXT_SIZE, old);
	return rec.y;
}
