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

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "pixel.h"

#define BUILDERWIDTH 32
struct {
	Shader shader;
	bool texidx;
	Texture textures[2];
	float positions[BUILDERWIDTH * BUILDERWIDTH * 2];
	uint16_t iteration;
} Builder;

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
}";

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

static char
		unused[CHUNK_WIDTH * BUILDERWIDTH * CHUNK_WIDTH * BUILDERWIDTH];

void initBuilder() {
	Image img = {0};
	img.data = unused;
	img.mipmaps = 1;
	img.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	img.width = CHUNK_WIDTH * BUILDERWIDTH;
	img.height = CHUNK_WIDTH * BUILDERWIDTH;
	Builder.textures[0] = LoadTextureFromImage(img);
	Builder.textures[1] = LoadTextureFromImage(img);
	if (!IsTextureReady(Builder.textures[0])) {
		perror("Can't make texture for TextureBuilder! Aborting...");
		abort();
	}
	Builder.shader = LoadShaderFromMemory(vertex, fragment);
	if (!IsShaderReady(Builder.shader)) abort();
	Builder.iteration = 0;
	Builder.texidx = true;
	SetTextureFilter(Builder.textures[0], TEXTURE_FILTER_POINT);
	SetTextureFilter(Builder.textures[1], TEXTURE_FILTER_POINT);
}

void freeBuilder() {
	UnloadTexture(Builder.textures[0]);
	UnloadTexture(Builder.textures[1]);
	UnloadShader(Builder.shader);
}

void flushChunksCache() {
	BeginShaderMode(Builder.shader);
	for (int pos = 0; pos < Builder.iteration; pos++) {
		int x = pos % BUILDERWIDTH;
		int y = pos / BUILDERWIDTH;
		DrawTextureRec(
				Builder.textures[Builder.texidx],
				(Rectangle){x * CHUNK_WIDTH, y * CHUNK_WIDTH, CHUNK_WIDTH,
										CHUNK_WIDTH},
				(Vector2){
						Builder.positions[x * 2 + y * BUILDERWIDTH * 2],
						Builder.positions[x * 2 + y * BUILDERWIDTH * 2 + 1]},
				WHITE);
	}
	Builder.iteration = 0;
	Builder.texidx = !Builder.texidx;
	EndShaderMode();
}

int renderChunk(struct chunk* c) {
	if (!c) return -1;
	Builder.positions[Builder.iteration * 2] =
			((int32_t)c->pos.axis[0]) * CHUNK_WIDTH;
	Builder.positions[Builder.iteration * 2 + 1] =
			((int32_t)c->pos.axis[1]) * CHUNK_WIDTH;
	int x = Builder.iteration % BUILDERWIDTH;
	int y = Builder.iteration / BUILDERWIDTH;
	UpdateTextureRec(Builder.textures[Builder.texidx],
									 (Rectangle){x * CHUNK_WIDTH, y * CHUNK_WIDTH,
															 CHUNK_WIDTH, CHUNK_WIDTH},
									 getChunkData(c, MODE_READ));
	Builder.iteration++;
	if (Builder.iteration >= BUILDERWIDTH * BUILDERWIDTH) {
		flushChunksCache();
	}
	return 0;
}
