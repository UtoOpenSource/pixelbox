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

namespace assets {

	void init();
	void free();
	void collect();

	typedef unsigned int AssetID;

	AssetID LookupID(const char* name);
	Texture GetTexture(AssetID id);
	const char* GetString(AssetID id);
	Wave GetWave(AssetID id);

	void GuiTexture(Rectangle rec, AssetID id);
	Sound PlaySound(AssetID id, float volume,
										 float pitch);	// already takes care!
	void CareSound(Sound sound);		// take care of existent sound
};
