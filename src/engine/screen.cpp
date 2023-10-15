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
		GuiUnlock();
		if (debug) rep |= debug->drawgui();
		if (rep) GuiLock();
		if (curr) rep = curr->drawgui();
		GuiUnlock();
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

extern "C" {

	int GuiWindowConf(GuiWindowCtx* x, const char* name, Rectangle bounds, bool moveable) {
		if (!x) return -1;
		x->hidden = 0;
		x->moveable = moveable;
		x->moving = 0;
		x->title = name ? name : "unnamed";
		x->rec = bounds;
		x->minw = x->rec.width;
		x->minh = x->rec.height;
		return 0;
	}

	int GuiWindow(GuiWindowCtx* ctx, winRedrawCallback cb) {
		GuiPanel(ctx->rec, ctx->title);
		Rectangle hrec = (Rectangle) { // hide button
			ctx->rec.x + ctx->rec.width - 20, ctx->rec.y + 2, 20, 20
		};

		int icon_kind = ctx->hidden ? ICON_ARROW_UP : ICON_ARROW_DOWN;
	
		if (GuiButton(hrec, GuiIconText(icon_kind, ""))) {
			ctx->hidden = ctx->hidden;
		}

		// content rectangle
		Rectangle bounds = (Rectangle) {
			ctx->rec.x+5, ctx->rec.y + 30,
			ctx->rec.width-10, ctx->rec.height - 35
		};

		if (!ctx->hidden && cb) cb(bounds); 
		return !ctx->hidden;
	}

	int GuiWindowCollision(GuiWindowCtx* ctx) {
		return (CheckCollisionPointRec(GetMousePosition(), ctx->rec));
	}

	// controversal!
	void GuiWindowMove(GuiWindowCtx* ctx) {
		Rectangle barrec = (Rectangle){
			ctx->rec.x, ctx->rec.y,
			ctx->rec.width, 20
		};

		if (ctx->moving && IsMouseButtonDown(0) && !GuiIsLocked()) {
			Vector2 delta = GetMouseDelta();
			ctx->rec.x += delta.x;
			ctx->rec.y += delta.y;
		} else
			ctx->moving = 0;

		if (CheckCollisionPointRec(GetMousePosition(), barrec) &&
				IsMouseButtonPressed(0)) {
				ctx->moving = 1;
		}
	}

	void GuiWindowCenter(GuiWindowCtx* ctx) {
		
	}
};
