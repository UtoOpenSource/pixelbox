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

#pragma once

#include "assets.h"
#include "raygui.h"
#include "raylib.h"

namespace screen {

	class Manager;

	class Base {
		public:
		class Base* prev = nullptr;
		Manager* manager = nullptr;
		public:
		Base() = default;
		virtual ~Base() = 0;
		virtual void update(float dt);
		virtual void drawback();
		virtual bool drawgui();
		virtual void shown();
		virtual void hidden();
	};

	class GuiCache {
		public:
		RenderTexture2D canvas;
		int w, h;
		double oldtime = 0.0;
		public:
		GuiCache();
		~GuiCache();
		void begin();
		void end();
	};

	class Manager {
		private :
		Base* curr = nullptr, *next = nullptr;
		Base* debug = nullptr;

		GuiCache* cache = nullptr;
		public :
		Manager() = default;
		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		~Manager();
		public:
		void update(double dt);
		void draw();
		public:
		void setNext(Base* screen);
		void setRoot(Base* screen);
		void setPrev(Base* fallback);
		public:
		void setDebug(Base* screen);
		Base* getDebug();
		void refreshGui(); // force redraw on next tick
		void release();
	};

};

extern "C" {
	
	struct GuiWindowCtx {
		Rectangle rec;
		int minw, minh;
		bool moveable;
		const char* title;
		// status
		bool moving;
		bool hidden;
	};

	int GuiWindowConf(GuiWindowCtx* x, const char* name, Rectangle bounds, bool movable);

	typedef void (*winRedrawCallback) (Rectangle bounds);

	int GuiWindow(GuiWindowCtx* ctx, winRedrawCallback cb);
	int GuiWindowCollision(GuiWindowCtx* ctx);

	// controversal!
	void GuiWindowMove(GuiWindowCtx* ctx);
	void GuiWindowCenter(GuiWindowCtx* ctx);

};

