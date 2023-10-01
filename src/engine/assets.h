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

void initAssetSystem();
void freeAssetSystem();
void collectAssets();

typedef unsigned int AssetID;

AssetID LookupAssetID(const char* name);
Texture GetTextureAsset(AssetID id);
const char* GetStringAsset(AssetID id);
Wave GetWaveAsset(AssetID id);

void GuiAssetTexture(Rectangle rec, AssetID id);
Sound PlayAssetSound(AssetID id, float volume,
										 float pitch);	// already takes care!
void TakeCareSound(Sound sound);		// take care of existent sound
