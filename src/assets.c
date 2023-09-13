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

#include "assets.h"
#include <stdlib.h>

enum {
	ASSET_NULL,
	ASSET_STRING,
	ASSET_TEXTURE,
	ASSET_AUDIO
};

#define USAGE_TIMEOUT 200
struct AssetNode {
	struct AssetNode* next;
	AssetID id;
	int type; int usage;
	union {
		Texture texture;
		char    string[1];
	} data;
};

#define HASH_LEN 64
static struct AssetNode* ASSET_MAP[HASH_LEN];
static Texture errorTexture;

#include "img_error.h"

void initAssetSystem() {
	for (int i = 0; i < HASH_LEN; i++)
		ASSET_MAP[i] = NULL; 

	Image err = (Image) {
		.data = img_error_data, 
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 
		.mipmaps = 1,
		.width = img_error_width,
		.height = img_error_height
	};
	errorTexture = LoadTextureFromImage(err);
}

static void free_node(struct AssetNode* n) {
	switch(n->type) {
		case ASSET_TEXTURE :
			UnloadTexture(n->data.texture);
		break;
		default :
		break;
	}
	free(n);
}

static struct _FileType {
	char str[4];
	int  type;
} fileTypes[] = {
	{"txt", ASSET_STRING},
	{"png", ASSET_TEXTURE},
	{"bmp", ASSET_TEXTURE},
	{"jpg", ASSET_TEXTURE},
	{"gif", ASSET_TEXTURE},
	{"", ASSET_NULL}
};

#include <string.h>

static const char* extension_pos(const char* path) {
	const char* ext = NULL, *p = path;
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
		if (p->str[0] == tmp[0] &&
			p->str[1] == tmp[1] &&
			p->str[2] == tmp[2]) return p->type;
	}
	return ASSET_NULL;
}

static inline unsigned rol(unsigned r, int k) {return (r << k) | (r >> (32 - k));}

static AssetID hash_string(const char* str) { // MurmurHash
	AssetID h = 3323198485ul;
  for (;*str;++str) {
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
			Texture2D texture = { 0 };

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

void freeAssetSystem() {
	for (int i = 0; i < HASH_LEN; i++) {
		struct AssetNode* n = ASSET_MAP[i];
		while (n) {
			struct AssetNode *f = n;
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
		struct AssetNode* n = ASSET_MAP[i], *old = NULL;
		while (n) {
			struct AssetNode *f = n;
			n = n->next;
			if (f->usage) f->usage--;
			if (f->usage || limit > 5) { 
				old = f;
			} else { // remove
				limit++;
				if (old) old->next = n;
				else ASSET_MAP[i] = n;
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
		if (i > 500) break; // oh no
		buff[i++] = c;
	}

	buff[i] = '\0';
}

#include <assert.h>

static struct AssetNode* getNode(AssetID id) {
	AssetID i = id & (HASH_LEN-1);
	struct AssetNode* n = ASSET_MAP[i];
	while (n) {
		if (n->id == id) {
			n->usage = USAGE_TIMEOUT;
			return n; // FOUND!
		}
		n = n->next;
	}
	return NULL;
}

AssetID LookupAssetID(const char* name) {
	makestr(name);
	AssetID id = hash_string(buff);
	AssetID i  = id & (HASH_LEN-1);

	if (getNode(id)) return id; // already exists!

	// else add new one...
	struct AssetNode *n = new_node(buff);
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
	if (!n || n->type != ASSET_TEXTURE)
		return errorTexture;
	return n->data.texture;
}

const char* GetStringAsset(AssetID id) {
	struct AssetNode* n = getNode(id);
	if (!n || n->type != ASSET_STRING)
		return "error";
	return n->data.string;
}
