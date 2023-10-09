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

#include "screen.h"
#include "engine.h"

namespace screen {

Base::~Base() {}
void Base::update(float dt) {}
void Base::drawback() {}
bool Base::drawgui() {return false;}
void Base::shown() {}
void Base::hidden() {}

// Gui Cache
GuiCache::GuiCache() {
	if (!IsWindowReady()) throw "can't draw when engine is not in GUI mode";

	w = GetScreenWidth();
	h = GetScreenHeight();
	canvas = ::LoadRenderTexture(w, h);
	oldtime = ::engine::getTime();
}
GuiCache::~GuiCache() {
	if (IsRenderTextureReady(canvas)) {
		UnloadRenderTexture(canvas);
	}
}
void GuiCache::begin() {
	if (w < GetScreenWidth() || h < GetScreenHeight()) {
		if (!IsWindowReady())
			throw "can't draw when engine is not in GUI mode";
		if (IsRenderTextureReady(canvas))
			UnloadRenderTexture(canvas);
		w = GetScreenWidth();
		h = GetScreenHeight();
		canvas = ::LoadRenderTexture(w, h);
	}

	if (!::IsRenderTextureReady(canvas)) return; // oh no
	::BeginTextureMode(canvas);
	::ClearBackground(Color{0, 0, 0, 0});
}

void GuiCache::end() {
	if (!::IsRenderTextureReady(canvas)) return; // oh no
	::EndTextureMode();
}

Manager::~Manager() {
	if (cache) {
		throw "you must release screen manager before freeing engine!";
	}
	if (curr) curr->hidden();
	if (debug) debug->hidden();
}

void Manager::release() {
	if (!IsWindowReady() && cache) {
		throw "you must release screen manager before freeing engine!";
	} 
	if (cache) {
		delete cache;
		cache = nullptr;
	}
}

void Manager::update(double dt) {
	if (next) { // oh no
		next->prev = curr;
		next->manager = this;
		if (curr) {
			curr->hidden();
		}
		curr = next;
		next->shown();
		next = nullptr;
	}

	if (debug) debug->update(dt);
	if (curr) curr->update(dt);
}

void Manager::refreshGui() {
	if (!cache) return;
	double time = ::engine::getTime(); 
	
	//if (cache->oldtime > time + 1.0/32.0)
	cache->oldtime = time;
}

void Manager::draw() {
	if (!cache) cache = new GuiCache();
	if (curr) curr->drawback();
	if (debug) debug->drawback();

	double t = ::engine::getTime();

	if (cache->oldtime < t) {
		cache->oldtime = t + 0.5;

		cache->begin();
		bool rep = false;
		if (curr) rep = curr->drawgui();
		if (debug) rep |= debug->drawgui();
		cache->end();

		double t = ::engine::getTime();

		if (rep) refreshGui();
	}

	::DrawText(TextFormat("time : %f", t), 0, 0, 20, RED);
	// draw precomputed one :D
	RenderTexture& tex = cache->canvas;
	DrawTextureRec(tex.texture, 
		(Rectangle){0,0,(float)tex.texture.width, (float)-tex.texture.height },
		Vector2{0, 0},
		WHITE
	);
}

void Manager::setNext(Base* s) {
	if (s && s != curr) {
		next = s;
	}
}

void Manager::setRoot(Base* s) {
	if (curr == s) return; // no
	if (curr) {
		curr->manager = this;
		curr->hidden();
	}
	curr = s;
	if (s) {
		curr->manager = this;
		curr->prev = nullptr;
		curr->shown();
	}
	next = nullptr;
}

void Manager::setPrev(Base* fb) {
	if (!curr || !curr->prev) {
		setRoot(fb); // ok
		return;
	}
	curr->manager = this;
	curr->hidden();
	curr = curr->prev;
	curr->manager = this;
	curr->shown();
}

void Manager::setDebug(Base* s) {
	if (debug) {
		debug->manager = this;
		debug->hidden();
	}	
	debug = s;
	if (debug) {
		debug->manager = this;
		debug->shown();
	}
}

Base* Manager::getDebug() {
	return debug;
}

};

