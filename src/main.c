// see pixelbox.h for copyright notice and license.

#include <raylib.h>
#include "pixel.h"
#include <stdio.h>

#define BUILDERWIDTH 8
struct {
	Shader   shader;
	Texture texture;
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

static void initBuilder() {
	Image img  = {0};
	img.data   = NULL;
	img.mipmaps = 1;
	img.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	img.width  = 32*BUILDERWIDTH;
	img.height = 32*BUILDERWIDTH;
	Builder.texture = LoadTextureFromImage(img);
	if (!IsTextureReady(Builder.texture)) abort();
	Builder.shader = LoadShaderFromMemory(0, fragment);
	if (!IsShaderReady(Builder.shader)) abort();
	Builder.iteration = 0;
	SetTextureFilter(Builder.texture, TEXTURE_FILTER_POINT);
}

static void freeBuilder() {
	UnloadTexture(Builder.texture);
	UnloadShader(Builder.shader);
}

static void flushChunksCache() {
	BeginShaderMode(Builder.shader);
	for (int pos = 0; pos < Builder.iteration; pos++) {
		int x = pos % BUILDERWIDTH;
		int y = pos / BUILDERWIDTH;
		DrawTextureRec(Builder.texture, (Rectangle){x*CHUNK_WIDTH, y*CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH}, 
			(Vector2){Builder.positions[x*2 + y*BUILDERWIDTH*2],
				Builder.positions[x*2 + y*BUILDERWIDTH*2 + 1]}, WHITE);
	}
	Builder.iteration = 0;
	EndShaderMode();
}

static int  renderChunk(struct chunk* c) {
	if (!c) return -1;
	Builder.positions[Builder.iteration * 2] =
		((int32_t)c->pos.axis[0]) * CHUNK_WIDTH;
	Builder.positions[Builder.iteration * 2 + 1] =
		((int32_t)c->pos.axis[1]) * CHUNK_WIDTH;
	int x = Builder.iteration % BUILDERWIDTH;
	int y = Builder.iteration / BUILDERWIDTH;
	UpdateTextureRec(Builder.texture, (Rectangle){x * CHUNK_WIDTH,
			y*CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH}, getChunkData(c, MODE_READ));
	Builder.iteration++;
	if (Builder.iteration >= BUILDERWIDTH*BUILDERWIDTH) {
			flushChunksCache();
	}
}


int main() {
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(640, 480, "[PixelBox] : amazing description");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	initBuilder();

	initWorld();
	openWorld(true ? ":memory:" : "test.db");
	Camera2D cam = {0};
	cam.zoom = 3;
	cam.rotation = 0;
	cam.target = (Vector2){120/2, 120/2};
	cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};

	while (!WindowShouldClose()) { 
		cam.offset = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};
		BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode2D(cam); // draw world
		if (IsMouseButtonDown(0)) {
			Vector2 md = GetMouseDelta();
			cam.target.x -= md.x /cam.zoom;
			cam.target.y -= md.y /cam.zoom;
		}

		cam.zoom += GetMouseWheelMove() * 0.15 * cam.zoom;
		if (cam.zoom < 0.01) cam.zoom = 0.01;
		if (cam.zoom > 100) cam.zoom = 100;
		
		int32_t x0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).x) / CHUNK_WIDTH - 1;
		int32_t x1 = (GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, cam).x) / CHUNK_WIDTH + 1;
		int32_t y0 = (GetScreenToWorld2D((Vector2){0, 0}, cam).y) / CHUNK_WIDTH - 1;
		int32_t y1 = (GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, cam).y) / CHUNK_WIDTH + 1;

		for (int32_t y = y0; y < y1; y++) {
			for (int32_t x = x0; x < x1; x++) {
				renderChunk(getWorldChunk(x, y));
			}
		}
		flushChunksCache();

		EndMode2D();
		DrawText("OK", 0, 0, 30, BLACK);
		EndDrawing();

		collectGarbage();
	};

	freeWorld();
	freeBuilder();
	CloseWindow();
	return 0;
}
