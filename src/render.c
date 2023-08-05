#include <raylib.h>
#include "pixel.h"
#include <stdio.h>

#define BUILDERWIDTH 16
struct {
	Shader   shader;
	bool texidx;
	Texture textures[2];
	float positions[BUILDERWIDTH*BUILDERWIDTH*2];
	uint16_t iteration;
} Builder;

static const char* fragment =
"#version 330 \n\
// Input vertex attributes (from vertex shader)\n\
in vec2 fragTexCoord;\n\
in vec4 fragColor;\n\
uniform sampler2D texture0;  \n\
out vec4 finalColor; \n\
\n\
vec4 effect(vec4 color, sampler2D tex, vec2 texture_coords) {\n\
	int val = int(texture(tex, texture_coords).r*255.0);\n\
	float kind = float(val & 3) / 6.0;\n\
	int type = (val >> 2) & 63;\n\
	float r  = float(type & 3) + kind;\n\
	float g  = float((type >> 2) & 3) + kind;\n\
	float b  = float((type >> 4) & 3) + kind;\n\
	return vec4(r/4.0, g/4.0, b/4.0, 1);\n\
	//return vec4(val/255.0, 1, 0, 1);\n\
}\n\
\n\
void main() {\n\
	finalColor = effect(fragColor, texture0, fragTexCoord);\n\
}"
;

void initBuilder() {
	Image img  = {0};
	img.data   = NULL;
	img.mipmaps = 1;
	img.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	img.width  = 32*BUILDERWIDTH;
	img.height = 32*BUILDERWIDTH;
	Builder.textures[0] = LoadTextureFromImage(img);
	Builder.textures[1] = LoadTextureFromImage(img);
	if (!IsTextureReady(Builder.textures[0])) abort();
	Builder.shader = LoadShaderFromMemory(0, fragment);
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
		DrawTextureRec(Builder.textures[Builder.texidx], (Rectangle){x*CHUNK_WIDTH, y*CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH}, 
			(Vector2){Builder.positions[x*2 + y*BUILDERWIDTH*2],
				Builder.positions[x*2 + y*BUILDERWIDTH*2 + 1]}, WHITE);
	}
	Builder.iteration = 0;
	Builder.texidx = !Builder.texidx;
	EndShaderMode();
}

int  renderChunk(struct chunk* c) {
	if (!c) return -1;
	Builder.positions[Builder.iteration * 2] =
		((int32_t)c->pos.axis[0]) * CHUNK_WIDTH;
	Builder.positions[Builder.iteration * 2 + 1] =
		((int32_t)c->pos.axis[1]) * CHUNK_WIDTH;
	int x = Builder.iteration % BUILDERWIDTH;
	int y = Builder.iteration / BUILDERWIDTH;
	UpdateTextureRec(Builder.textures[Builder.texidx], (Rectangle){x * CHUNK_WIDTH,
			y*CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH}, getChunkData(c, MODE_READ));
	Builder.iteration++;
	if (Builder.iteration >= BUILDERWIDTH*BUILDERWIDTH) {
			flushChunksCache();
	}
}

