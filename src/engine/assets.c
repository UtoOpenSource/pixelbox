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

#include "assets.h"

#include <stdlib.h>

enum { ASSET_NULL, ASSET_STRING, ASSET_TEXTURE, ASSET_WAVE };

#define USAGE_TIMEOUT 200
struct AssetNode {
	struct AssetNode* next;
	AssetID id;
	int type;
	int usage;
	union {
		Texture texture;
		Wave wave;
		char string[1];
	} data;
};

#define HASH_LEN 64
static struct AssetNode* ASSET_MAP[HASH_LEN];
static Texture errorTexture;
static Wave errorWave;
static char null_data[128] = {0};

#include "img_error.h"

// DEFLATE'd!
extern const struct archive_node {
	const char* name;
	const long unsigned int length;
	const unsigned char* value;
} __main_arcive[];

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRACELOG(...) TraceLog(__VA_ARGS__);

static uint8_t* loadFile(const char* fn, unsigned int* cnt) {
	for (const struct archive_node* n = __main_arcive; n->name; n++) {
		if (strcmp(fn, n->name) == 0) {
			// char* copy = malloc(n->length+1);
			// if (!copy) return NULL;
			// memcpy(copy, n->value, n->length);
			int ressize = 0;
			uint8_t* res =
					DecompressData((uint8_t*)n->value, n->length, &ressize);
			*cnt = ressize;
			return res;
		}
	}
	// else
	// taken form raylib, else i will actually write same stuff...
	FILE* file = fopen(fn, "rb");
	char* data = NULL;

	if (file != NULL) {
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (size > 0) {
			data = (unsigned char*)malloc(size + 1);
			long int count = fread(data, sizeof(unsigned char), size, file);
			*cnt = count;
		} else
			TRACELOG(LOG_WARNING, "Failed to read file %s", fn);
		fclose(file);
	} else
		TRACELOG(LOG_WARNING, "Failed to open file %s", fn);
	*cnt = 0;
	return NULL;
}

static char* loadFileT(const char* fn) {
	unsigned int cnt = 0;
	uint8_t* dat = loadFile(fn, &cnt);
	// check for trailing zero
	if (dat && dat[cnt] != 0) {
		dat[cnt] = 0;
	}
	return (char*)dat;	// i don't care...
}

void initAssetSystem() {
	for (int i = 0; i < HASH_LEN; i++) ASSET_MAP[i] = NULL;

	SetLoadFileDataCallback(loadFile);
	SetLoadFileTextCallback(loadFileT);

	Image err = (Image){.data = img_error_data,
											.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
											.mipmaps = 1,
											.width = img_error_width,
											.height = img_error_height};
	errorTexture = LoadTextureFromImage(err);
	errorWave = (Wave){1, 1, 8, 1, null_data};
}

static void free_node(struct AssetNode* n) {
	switch (n->type) {
		case ASSET_TEXTURE:
			UnloadTexture(n->data.texture);
			break;
		case ASSET_WAVE:
			UnloadWave(n->data.wave);
			break;
		default:
			break;
	}
	free(n);
}

static struct _FileType {
	char str[4];
	int type;
} fileTypes[] = {{"txt", ASSET_STRING},	 {"png", ASSET_TEXTURE},
								 {"bmp", ASSET_TEXTURE}, {"jpg", ASSET_TEXTURE},
								 {"gif", ASSET_TEXTURE}, {"wav", ASSET_WAVE},
								 {"ogg", ASSET_WAVE},		 {"", ASSET_NULL}};

#include <string.h>

static const char* extension_pos(const char* path) {
	const char *ext = NULL, *p = path;
	while (*p) {
		if (*p == '.') ext = p + 1;
		p++;
	}
	return ext;
}

static int check_type(const char* ext) {
	if (!ext) return ASSET_NULL;
	int len = strlen(ext);
	if (len < 3) return ASSET_NULL;

	char tmp[4];
	struct _FileType* p = fileTypes;

	memcpy(tmp, ext, 3);
	tmp[3] = 0;

	for (; p->type != ASSET_NULL; p++) {
		if (p->str[0] == tmp[0] && p->str[1] == tmp[1] &&
				p->str[2] == tmp[2])
			return p->type;
	}
	return ASSET_NULL;
}

static inline unsigned rol(unsigned r, int k) {
	return (r << k) | (r >> (32 - k));
}

static AssetID hash_string(const char* str) {	 // MurmurHash
	AssetID h = 3323198485ul;
	for (; *str; ++str) {
		h ^= *str;
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}

	if (h == (AssetID)(-1)) h++;
	return h;
}

#include "profiler.h"

static struct AssetNode* new_node(const char* path) {
	const char* ext = extension_pos(path);
	int type = check_type(ext);
	struct AssetNode* node = NULL;

	switch (type) {
		case ASSET_STRING: {
			prof_begin(PROF_DISK);
			char* data = LoadFileText(path);
			prof_end();

			if (!data) break;
			int len = strlen(data) + 1;

			node = calloc(sizeof(struct AssetNode) + len, 1);
			if (!node) {
				UnloadFileText(data);
				break;
			}

			node->type = ASSET_STRING;
			memcpy(node->data.string, data, len);
			UnloadFileText(data);
		} break;
		case ASSET_TEXTURE: {
			Texture2D texture = {0};

			prof_begin(PROF_DISK);
			Image img = LoadImage(path);
			prof_end();

			if (img.data == NULL) break;
			texture = LoadTextureFromImage(img);
			UnloadImage(img);

			node = calloc(sizeof(struct AssetNode), 1);
			if (!node) {
				UnloadTexture(texture);
				break;
			}

			node->type = ASSET_TEXTURE;
			node->data.texture = texture;
		} break;
		case ASSET_WAVE: {
			prof_begin(PROF_DISK);
			Wave wave = LoadWave(path);
			prof_end();

			if (!IsWaveReady(wave)) break;

			node = calloc(sizeof(struct AssetNode), 1);
			if (!node) {
				UnloadWave(wave);
				break;
			}

			node->type = ASSET_WAVE;
			node->data.wave = wave;
		} break;
	};

	if (!node) {
		node = calloc(sizeof(struct AssetNode), 1);
		if (!node) return NULL;
		node->type = ASSET_NULL;
	}
	node->id = hash_string(path);
	node->usage = USAGE_TIMEOUT;
	return node;
}

static void UnloadSounds();
void freeAssetSystem() {
	UnloadSounds();
	for (int i = 0; i < HASH_LEN; i++) {
		struct AssetNode* n = ASSET_MAP[i];
		while (n) {
			struct AssetNode* f = n;
			n = n->next;
			free_node(f);
		}
		ASSET_MAP[i] = NULL;
	}
	UnloadTexture(errorTexture);
}

void collectAssets() {
	int limit = 0;

	for (int i = 0; i < HASH_LEN; i++) {
		struct AssetNode *n = ASSET_MAP[i], *old = NULL;
		while (n) {
			struct AssetNode* f = n;
			n = n->next;
			if (f->usage) f->usage--;
			if (f->usage || limit > 5) {
				old = f;
			} else {	// remove
				limit++;
				if (old)
					old->next = n;
				else
					ASSET_MAP[i] = n;
				free_node(f);
			}
		}
	}
}

static char buff[512] = {0};
static void makestr(const char* str) {
	int c, i = 0;

	while ((c = *str++) != '\0') {
		if (c == ' ') continue;
		if (c == '/' && *str == '/') continue;
		if (i > 500) break;	 // oh no
		buff[i++] = c;
	}

	buff[i] = '\0';
}

#include <assert.h>

static struct AssetNode* getNode(AssetID id) {
	AssetID i = id & (HASH_LEN - 1);
	struct AssetNode* n = ASSET_MAP[i];
	while (n) {
		if (n->id == id) {
			n->usage = USAGE_TIMEOUT;
			return n;	 // FOUND!
		}
		n = n->next;
	}
	return NULL;
}

AssetID LookupAssetID(const char* name) {
	makestr(name);
	AssetID id = hash_string(buff);
	AssetID i = id & (HASH_LEN - 1);

	if (getNode(id)) return id;	 // already exists!

	// else add new one...
	struct AssetNode* n = new_node(buff);
	if (n) {
		assert(n->id == id);
		n->next = ASSET_MAP[i];
		ASSET_MAP[i] = n;
		return id;
	}

	// everything failed!
	return (AssetID)(-1);
}

Texture GetTextureAsset(AssetID id) {
	struct AssetNode* n = getNode(id);
	if (!n || n->type != ASSET_TEXTURE) return errorTexture;
	return n->data.texture;
}

const char* GetStringAsset(AssetID id) {
	struct AssetNode* n = getNode(id);
	if (!n || n->type != ASSET_STRING) return "error";
	return n->data.string;
}

Wave GetWaveAsset(AssetID id) {
	struct AssetNode* n = getNode(id);
	if (!n || n->type != ASSET_WAVE) return errorWave;
	return n->data.wave;
}

#define SOUND_PULL_SIZE 20
static Sound sound_pull[SOUND_PULL_SIZE] = {0};
static int free_sound = 0;

static void UnloadSounds() {
	for (int i = 0; i < SOUND_PULL_SIZE; i++) {
		if (IsSoundReady(sound_pull[i])) {
			UnloadSound(sound_pull[i]);
			sound_pull[i] = (Sound){0};
		}
	}

	free_sound = 0;
}

static Sound* getSpace() {
	for (int i = free_sound; i < SOUND_PULL_SIZE; i++) {
		if (IsSoundReady(sound_pull[i])) {
			if (!IsSoundPlaying(sound_pull[i])) {
				UnloadSound(sound_pull[i]);
				sound_pull[i] = (Sound){0};
				free_sound = i + 1;
				return sound_pull + i;
			}
			free_sound++;
			continue;
		}
		return sound_pull + i;
	}

	// try to find expired sound
	for (int i = 0; i < SOUND_PULL_SIZE; i++) {
		if (IsSoundReady(sound_pull[i]) && IsSoundPlaying(sound_pull[i]))
			continue;
		// found!
		UnloadSound(sound_pull[i]);
		sound_pull[i] = (Sound){0};
		free_sound = i + 1;
		return sound_pull + i;
	}

	// no free sound :( remove old one
	UnloadSound(sound_pull[0]);
	for (int i = 0; i < SOUND_PULL_SIZE - 1; i++) {
		sound_pull[i] = sound_pull[i + 1];
	}
	sound_pull[SOUND_PULL_SIZE - 1] = (Sound){0};
	return sound_pull + (SOUND_PULL_SIZE - 1);
}

void TakeCareSound(Sound sound) { *getSpace() = sound; }

Sound PlayAssetSound(AssetID id, float volume, float pitch) {
	Sound snd = LoadSoundFromWave(GetWaveAsset(id));
	assert(IsSoundReady(snd));
	SetSoundVolume(snd, volume);
	SetSoundPitch(snd, pitch);
	PlaySound(snd);
	TakeCareSound(snd);
	return snd;
}

#include "raygui.h"

void GuiAssetTexture(Rectangle rec, AssetID id) {
	Texture tex = GetTextureAsset(id);

	DrawTexturePro(tex, (Rectangle){0, 0, tex.width, tex.height}, rec,
								 (Vector2){0, 0}, 0, WHITE);
}
