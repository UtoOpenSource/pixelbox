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

#include <raylib.h>
#include "pixel.h"
#include "implix.h"
#include <stdlib.h>
#include <stdio.h>

static const char* fragment =
#ifdef PLATFORM_WEB
"#version 300 es\n\
precision mediump float;"
#else
"#version 330\n"
#endif
"// Input vertex attributes (from vertex shader)\n\
in vec2 fragTexCoord;\n\
in vec4 fragColor;\n\
uniform sampler2D texture0;  \n\
out vec4 finalColor; \n\
\n\
vec4 effect(vec4 color, sampler2D tex, vec2 texture_coords) {\n\
	int val = int(texture(tex, texture_coords).r*255.1);\n\
	float kind = float(val & 3)/6.0;\n\
	int type = (val >> 2) & 63;\n\
	float r  = float(type & 3) + kind;\n\
	float g  = float((type >> 2) & 3) + kind;\n\
	float b  = float((type >> 4) & 3) + kind;\n\
	return vec4(r/4.0, g/4.0, b/4.0, 1);\n\
}\n\
\n\
void main() {\n\
	finalColor = effect(fragColor, texture0, fragTexCoord);\n\
}"
;

static const char* vertex = 
#ifdef PLATFORM_WEB
"#version 300 es\n\
precision mediump float;\n\
in vec3 vertexPosition;\n\
in vec2 vertexTexCoord;\n\
in vec3 vertexNormal;\n\
in vec4 vertexColor;\n\
uniform mat4 mvp;\n\
out vec2 fragTexCoord;\n\
out vec4 fragColor;\n\
void main()\n\
{\n\
    fragTexCoord = vertexTexCoord;\n\
    fragColor = vertexColor;\n\
    gl_Position = mvp*vec4(vertexPosition, 1.0);\n\
}"
#else
NULL
#endif
;

#define BUILDERWIDTH 64
#define RENDER_MAX   BUILDERWIDTH*BUILDERWIDTH

struct gitem { // graphical item (chunk)
	union packpos pos;
	struct gitem* next;
	bool used;
};

struct {
	Shader  shader;
	Texture texture;
	struct gitem items[RENDER_MAX];
	struct gitem* map[MAPLEN];
	uint16_t freeitem; // index of free item
	uint16_t requests;
} Builder;

static char unused[CHUNK_WIDTH*BUILDERWIDTH*CHUNK_WIDTH*BUILDERWIDTH] = {0};

void initBuilder() {
	Image img  = {0};
	img.data   = unused;
	img.mipmaps = 1;
	img.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	img.width  = CHUNK_WIDTH*BUILDERWIDTH;
	img.height = CHUNK_WIDTH*BUILDERWIDTH;
	Builder.texture = LoadTextureFromImage(img);

	if (!IsTextureReady(Builder.texture)) {
		perror("Can't make texture for TextureBuilder! Aborting...");
		abort();
	}

	Builder.shader = LoadShaderFromMemory(vertex, fragment);
	if (!IsShaderReady(Builder.shader)) abort();
	SetTextureFilter(Builder.texture, TEXTURE_FILTER_POINT);
}

void freeBuilder() {
	UnloadTexture(Builder.texture);
	UnloadShader(Builder.shader);
	for (int i = 0; i < RENDER_MAX; i++)
		Builder.items[i].used = false;
	for (int i = 0; i < MAPLEN; i++)
		Builder.map[i] = NULL;
	Builder.freeitem = 0;
}

static bool collides(struct gitem* o, int32_t x, int32_t y, int32_t x2, int32_t y2) {
	return (
		(int32_t)o->pos.axis[0] <= x2 &&
    (int32_t)o->pos.axis[0] >= x  &&
    (int32_t)o->pos.axis[1] <= y2 &&
    (int32_t)o->pos.axis[1] >= y
	);
}

#include "game.h"

void debugRender(Rectangle rec) {
	for (int i = 0; i < RENDER_MAX; i++) {
			int x = i % BUILDERWIDTH;
			int y = i / BUILDERWIDTH;
			struct gitem* o = Builder.items + i;
			DrawPixel(rec.x + x, rec.y + y, o->used ? (GREEN) : GRAY);
	}

	int x = Builder.freeitem % BUILDERWIDTH;
	int y = Builder.freeitem / BUILDERWIDTH;
	DrawPixel(rec.x + x, rec.y + y, YELLOW);

	rec.x += BUILDERWIDTH + 5;
	for (int i = 0; i < MAPLEN; i++) {
		struct gitem *o = Builder.map[i];
		int j = 0;
		while (o) {
			DrawPixel(rec.x + j, rec.y + i, PINK);
			o = o->next;
			j++;
		}
	}
}

// manip
static struct gitem* findItem(union packpos pos) {
	uint16_t i = MAPHASH(pos.pack);
	struct gitem* o = Builder.map[i];
	
	while (o) {
		if (o->pos.pack == pos.pack) return o;
		o = o->next;
	}

	return NULL;
}

struct gitem* newItem(union packpos pos) {
	repeat:
	if (Builder.freeitem >= RENDER_MAX) return NULL;
	struct gitem* o = Builder.items + (Builder.freeitem++);
	if (o->used) goto repeat;
	o->used = 1;
	o->pos.pack = pos.pack;

	uint16_t i = MAPHASH(pos.pack);
	o->next = Builder.map[i];
	Builder.map[i] = o;
	return o;
}

static struct gitem* removeItem(struct gitem* f) {
	if (!f) return NULL;
	uint16_t i = MAPHASH(f->pos.pack);	
	struct gitem* o = Builder.map[i], *p = NULL;

	while (o) {
		if (o == f) {
			if (p) p->next = o->next;
			else Builder.map[i] = o->next;
			o->used = false;
			uint16_t idx = (uint16_t)(o - Builder.items);
			if (idx < Builder.freeitem) Builder.freeitem = idx;
			return o->next;
		};
		p = o;
		o = o->next;
	}
	perror("CAn'T ERMOVE!");
	return NULL;
}

static void updateData(int index, struct chunk* c) {
	int x = index % BUILDERWIDTH;
	int y = index / BUILDERWIDTH;
	UpdateTextureRec(
		Builder.texture,
		(Rectangle) {
			x * CHUNK_WIDTH,
			y*CHUNK_WIDTH,
			CHUNK_WIDTH, CHUNK_WIDTH
		},
		getChunkAtoms(c, MODE_READ)->types
	);
}

#include <assert.h>

static void collectItems(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	for (int i = 0; i < MAPLEN; i++) {
		struct gitem *o = Builder.map[i];
		while (o) {
			assert(o->used && "render hashmap corrupted!");
			// DEBUG
			if (!collides(o, x0, y0, x1, y1)) {
				 o = removeItem(o); // hehe
			}
			else o = o->next;
		}
	}
}

static struct gitem* getItem(union packpos pos) {
	struct gitem* o = findItem(pos);

	if (o) { // founded! do some important stuff...

		// fast path
		struct chunk* c = findChunk(&World.map, pos.axis[0], pos.axis[1]);
		assert(c != NULL);
		/*{ // oh fuck!
			removeItem(o);
			Builder.requests--;
			return NULL; 
		}*/
		c->usagefactor = CHUNK_USAGE_VALUE;
		
		if (c->is_changed) {
			updateData((int)(o - Builder.items), c); // nice
		}
		return o;
	}

	// else create new guy!
	Builder.requests++;
	if (Builder.requests >= RENDER_MAX) return NULL; // ok

	struct chunk* c;

	c = getWorldChunk(pos.axis[0], pos.axis[1]);
	if (c == &empty) { // oh man...
		return NULL;
	}

	o = newItem(pos);
	if (!o) return NULL; // should not happen

	updateData((int)(o - Builder.items), c);
	return o;
}

#define swap(a, b) {do {int t = a; a = b; b = t;} while(0);}

void updateRender(Camera2D cam) {
	Builder.requests = Builder.freeitem;
	
	// get rectangle
	int64_t x0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).x)/ CHUNK_WIDTH - 1;
	int64_t x1 = (GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, cam).x) / CHUNK_WIDTH;
	int64_t y0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).y) / CHUNK_WIDTH - 1;
	int64_t y1 = (GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, cam).y) / CHUNK_WIDTH;
	if (x1 < x0) swap(x1, x0);
	if (y1 < y0) swap(y1, y0);

	// collect garbage :З
	collectItems(x0, y0, x1, y1);

	// add chunks in visible range
	for (int64_t y = y0; y <= y1; y++) {
		for (int64_t x = x0; x <= x1; x++) {
			union packpos pos;
			pos.axis[0] = x;
			pos.axis[1] = y;
			struct gitem* o = NULL;

			o = getItem(pos);

			float rx = x * CHUNK_WIDTH;
			float ry = y * CHUNK_WIDTH;

			if (!o) {
				DrawRectangleRec(
					(Rectangle){rx, ry, CHUNK_WIDTH, CHUNK_WIDTH},
					getPixelColor(softGenerate(x, y))
				);
				continue;
			} 

			// else if this is valid chunk... do nothing
		}
	}

	BeginShaderMode(Builder.shader);
	for (int i = 0; i < MAPLEN; i++) {
		struct gitem *o = Builder.map[i];
		while (o) {
			int i = (int)(o - Builder.items);
			int x = i % BUILDERWIDTH;
			int y = i / BUILDERWIDTH;
			assert(collides(o, x0, y0, x1, y1));
			DrawTextureRec(
					Builder.texture,
					(Rectangle) {
						x*CHUNK_WIDTH,
						y*CHUNK_WIDTH,
						CHUNK_WIDTH,
						CHUNK_WIDTH
					}, 
					(Vector2) {
						o->pos.axis[0] * CHUNK_WIDTH,
						o->pos.axis[1] * CHUNK_WIDTH
					}, WHITE);
			o = o->next;
		}
	}
	EndShaderMode();

}


