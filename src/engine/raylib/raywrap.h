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

// raylib wrapper

#include "raylib.h"
#pragma once
#include "raygui.h"
#include <memory>

namespace rl {

	class Object {
		public:
		Object() = default;
		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		virtual ~Object() = 0;
		virtual void unload(); // unload graphical stuff
	};

	void init(); // init raylib
	void free();

	class Image : public Object {
		public:
		::Image img;
		Image() : img({nullptr, 0, 0, 1, 0}) {};
		Image(const char* path); // extra
		~Image() {
			if (IsImageReady(img))
				UnloadImage(img);
		}
		operator bool() {
			return IsImageReady(img);
		}
	};

	class Texture : public Object {
		public:
		::Texture tex;
		void release() { // release owning
			tex = {0, 0, 0, 1, 0};
		}
		Texture() {release();};
		Texture(Image& image) {
			tex = LoadTextureFromImage(image.img);
		}
		Texture(const char* path); // extra
		~Texture() {
			if (IsTextureReady(tex))
				UnloadTexture(tex);
		}
		operator bool() {
			return IsTextureReady(tex);
		}
		operator ::Texture() {
			return tex;
		}
	};

	class RenderTexture : public Object {
		public:
		::RenderTexture tex;
		RenderTexture(int w, int h) {
			tex = LoadRenderTexture(w, h);
		}
		RenderTexture() : RenderTexture(GetScreenWidth(), GetScreenHeight()) {}
		~RenderTexture() {
			if (IsRenderTextureReady(tex))
				UnloadRenderTexture(tex);
		}
		operator bool() {
			return IsRenderTextureReady(tex);
		}
		operator ::Texture () {
			return tex.texture;
		}
	};

	class Font : public Object {
		public:
		::Font font;
		void release() { // release owning
			font = {0, 0, 0, {0}, nullptr, nullptr};
		}
		Font() {
			release();
		}
		Font(const char*); // ext
		~Font() {
			if (IsFontReady(font))
				UnloadFont(font);
		}
		operator bool() {
			return IsFontReady(font);
		}
		operator ::Font () {
			return font;
		}
	};

};


