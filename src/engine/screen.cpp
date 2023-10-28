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

static Rectangle back_rec = Rectangle{5, 5, 64, 25};

void Manager::update(double dt) {
	if (next) { // oh no
		next->prev = curr;
		next->manager = this;
		if (curr) {
			curr->hidden();
		}
		curr = next;
		curr->shown();
		next = nullptr;
	}

	if ( curr && curr->prev && 
		CheckCollisionPointRec(GetMousePosition(), back_rec)
	) {
		this->refreshGui();
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
	
		if (curr && curr->prev) {
			if (GuiButton(Rectangle{5, 5, 64, 25}, "< Back")) {
				this->setPrev(nullptr);
			}
			if (CheckCollisionPointRec(GetMousePosition(), back_rec)) {
				rep |= 1;
			}
		}

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

	void GuiWindowImpl(GuiWindowCtx* ctx) {
		Rectangle real = ctx->rec;
		if (ctx->hidden) real.height = 20;

		Rectangle hrec = (Rectangle) { // hide button
			ctx->rec.x + ctx->rec.width - 20, ctx->rec.y + 2, 20, 20
		};

		int icon_kind = ctx->hidden ? ICON_ARROW_UP : ICON_ARROW_DOWN;

		if (GuiButton(hrec, GuiIconText(icon_kind, ""))) {
			ctx->hidden = !ctx->hidden;
		}
	}

	int GuiWindow(GuiWindowCtx* ctx, std::function<void(Rectangle)> cb) {
		Rectangle real = ctx->rec;
		if (ctx->hidden) real.height = 20;

		GuiPanel(real, ctx->title);

		// content rectangle
		Rectangle bounds = (Rectangle) {
			ctx->rec.x+5, ctx->rec.y + 25,
			ctx->rec.width-10, ctx->rec.height - 30
		};

		if (!ctx->hidden && cb) cb(bounds); 
		GuiWindowImpl(ctx);
		return !ctx->hidden;
	}

	int GuiWindowCollision(GuiWindowCtx* ctx) {
		Rectangle real = ctx->rec;
		if (ctx->hidden) real.height = 20;
		return (CheckCollisionPointRec(GetMousePosition(), real));
	}

	// controversal!
	void GuiWindowMove(GuiWindowCtx* ctx) {
		Rectangle barrec = (Rectangle){
			ctx->rec.x, ctx->rec.y,
			ctx->rec.width, 20
		};

		Rectangle real = ctx->rec;
		if (ctx->hidden) real.height = 20;

		if (ctx->moving && IsMouseButtonDown(0) && !GuiIsLocked()) {
			Vector2 delta = GetMouseDelta();
			ctx->rec.x += delta.x;
			ctx->rec.y += delta.y;
		} else
			ctx->moving = 0;

		// bounds checking
		if (ctx->rec.x < 0) ctx->rec.x = 0;
		if (ctx->rec.y < 0) ctx->rec.y = 0;
		if (ctx->rec.x + ctx->rec.width > GetScreenWidth())
			ctx->rec.x = GetScreenWidth() - ctx->rec.width;
		if (ctx->rec.y + real.height > GetScreenHeight())
			ctx->rec.y = GetScreenHeight() - real.height;


		if (CheckCollisionPointRec(GetMousePosition(), barrec) &&
				IsMouseButtonPressed(0)) {
				ctx->moving = 1;
		}
	}

	void GuiWindowCenter(GuiWindowCtx* ctx) {
		float x = GetScreenWidth() / 5.0;
		float y = GetScreenHeight() / 5.0;
		float w = x * 4;
		float h = y * 4;

		// redo
		if (!ctx->moveable || (ctx->minh < h || ctx->minw < w)) {
			float hw = GetScreenWidth()/2.0;	
			float hh = GetScreenHeight()/2.0;	
			w = ctx->minw;
			h = ctx->minh;
			x = hw - w/2.0;
			y = hh - h/2.0;
		}

		ctx->rec = (Rectangle) {
			x, y, w, h
		};
	}
